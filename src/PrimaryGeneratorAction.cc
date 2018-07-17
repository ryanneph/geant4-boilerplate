#include "PrimaryGeneratorAction.hh"

#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "G4RunManager.hh"
#include "G4WorkerRunManager.hh"
#include "Randomize.hh"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include "G4PrimaryParticle.hh"
#include "G4Threading.hh"

// from ../main.cc
extern long int g_eventsProcessed;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(),
    fParticleGun(0)
{
    init();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // reproducible seeding for each event based on event number per suggestion from:
    // http://hypernews.slac.stanford.edu/HyperNews/geant4/get/runmanage/264/1.html?inline=-1
    // This is a multi-threading friendly approach and should give independent sampling for up to 900,000,000 events (HepJames pRNG seed range)
    // WARNING: EventID resets to 0 for every run; to handle this, we calculate continuous EventID assuming all runs have same number of events.
    //   If this assumption is broken, need to switch to a global counter that is updated by master thread after each run instead
    // auto* run = G4RunManager::GetRunManager()->GetCurrentRun();
    // long int globalEventID = g_eventsProcessed + anEvent->GetEventID()+1;
    // G4Random::setTheSeed(globalEventID);
    // G4cout << "Run #" << run->GetRunID() << ", EventID: " << anEvent->GetEventID() << " (global EventID: "<<globalEventID<<"), seed: " << G4Random::getTheSeed() << " (RNG ptr: "<<(void*)G4Random::getTheEngine()<<")" << G4endl;
    generate(anEvent);
}


#ifndef USEPHASESPACE
void PrimaryGeneratorAction::init() {
    fParticleGun = new G4GeneralParticleSource();
}
void PrimaryGeneratorAction::generate(G4Event* anEvent) {
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
/* ############################################################################# */
#else

void PrimaryGeneratorAction::init() {
    fParticleGun = new G4ParticleGun();
    fPlaceholder = 0; //placeholder for where we are in the file
    fPT = G4ParticleTable::GetParticleTable();

#ifdef G4MULTITHREADED
    int nthreads = G4Threading::GetNumberOfRunningWorkerThreads();
    int threadid = G4Threading::G4GetThreadId();
    int phsp_per_thread = NUM_PHSP_FILES/nthreads;

    // G4cout << "using multi-threaded PrimaryGeneratorAction with " << nthreads << " threads on thread " << threadid << G4endl;
    for (int i=NUM_PHSP_FILES-1-((NUM_THREADS-1-threadid)*phsp_per_thread); i>=threadid*phsp_per_thread; i--) {
        std::ostringstream this_path;
        this_path << "./PSF/TrueBeam_v2_6FFF_" << std::setw(2) << std::setfill('0') << i << ".IAEAphsp";
        // G4cout << "Adding \"" << this_path.str() << "\" to psf vector" << G4endl;
        psfvects.psFiles(threadid).push_back(this_path.str());
    }
#else
    int threadid = -2;
    // G4cout << "using single threaded PrimaryGeneratorAction" << G4endl;
    for (int i=15; i>=0; i--) {
        std::ostringstream this_path;
        this_path << "./PSF/TrueBeam_v2_6FFF_" << std::setw(2) << std::setfill('0') << i << ".IAEAphsp";
        // G4cout << "Adding \"" << this_path.str() << "\" to psf vector" << G4endl;
        psfvects.psFiles(threadid).push_back(this_path.str());
    }
#endif
}
void PrimaryGeneratorAction::generate(G4Event* anEvent) {
    G4ParticleDefinition *aParticle;
    //if out of particles, get more
    if (pList.size() == 0) {
        BatchHistories();
    }
    if (pList.size() == 0) {
        G4cout << "ERROR: BatchHistories() returned no new particles. ABORTING RUN" << G4endl;
        G4RunManager* runManager = G4RunManager::GetRunManager();
        runManager->AbortRun();
    }

    pspinfo pp = pList.front();
    pList.pop_front();

    if (pp.t == 1) aParticle = fPT->FindParticle("gamma");
    if (pp.t == 2) aParticle = fPT->FindParticle("e-");
    if (pp.t == 3) aParticle = fPT->FindParticle("e+");
    fParticleGun->SetParticleDefinition(aParticle);
    fParticleGun->SetParticlePosition(G4ThreeVector(pp.X, pp.Y, pp.Z));
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(pp.U,pp.V,pp.W));
    fParticleGun->SetParticleEnergy(pp.E);
    fParticleGun->GeneratePrimaryVertex(anEvent);
}

bool debugpsf = false;
void PrimaryGeneratorAction::BatchHistories()
{
    int threadid = G4Threading::G4GetThreadId();
    int batchSize = default_batchSize;
    int lineSize = 21; // must match sum of included data length [bytes]

    char pType;				//1 byte particle type
    G4float E, X, Y, U, V;  //4 byte floating points
    G4float Z, W;			//not included in binary file

    // any psf's left check
    if (psfvects.psFiles(threadid).size() <= 0 ) {
        G4cout << "PSF: ERROR: No phase space files remaining" << G4endl;
        return;
    }
    std::string psfile = psfvects.psFiles(threadid).back();
    if (fPlaceholder == 0) {
        G4cout << "PSF: Beginning new phase space file: \"" << psfvects.psFiles(threadid).back() << "\"" << G4endl;
    }

    // file open check
    std::ifstream infile(psfile, std::ios::in | std::ios::binary);
    if (infile.fail()) {
        infile.close();
        G4cout << "PSF: WARNING: Couldn't open file: \"" << psfile << "\". Trying next phase space file" << G4endl;
        psfvects.psFiles(threadid).pop_back();
        fPlaceholder = 0;
        BatchHistories();
        return;
    }

    // Check if we are nearing the end of this file
    infile.seekg(0, infile.end);
    int flen = infile.tellg()/lineSize;
    if (infile.eof() || fPlaceholder >= flen*lineSize) {
        // we should have switched to new file in previous batch read. this is an error that we shouldn't ever see
        infile.close();
        G4cout << "PSF: UNHANDLED ERROR: We have reached the end of this phase space file" << G4endl;
        return;
    }

    // prevent overreading the file
    else if (flen-(fPlaceholder/lineSize) < batchSize) {
        batchSize = flen-(fPlaceholder/lineSize);
        if (debugpsf)
            G4cout << "PSF: Nearing end of file, reducing batchSize to " << batchSize << G4endl;
    }
    infile.seekg(fPlaceholder, infile.beg); //from beginning of file, skip to where we last left off

    if (debugpsf) {
        G4cout << "PSF: Loading Batch of " << batchSize << " particles ("<<(fPlaceholder/lineSize)+1 << "->"<<(fPlaceholder/lineSize)+batchSize<<" of "<<flen<<") from file \"" << psfile << "\":" << G4endl;
    }
    for(G4int i=0;i<batchSize; i++) {
        pspinfo temp = {};

        infile.read((char*)&pType, 1);
        infile.read((char*)&E, 4);
        infile.read((char*)&X, 4);
        infile.read((char*)&Y, 4);
        infile.read((char*)&U, 4);
        infile.read((char*)&V, 4);

        // Z = 26.7*cm;//from header.  how silly.
        // Z = Z - 100 * cm;//Z is specified as distance away from target/source.  Get Z in terms of distance from iso, with Z_iso=0;
        Z = -50*cm; // Prescribed position
        W = sqrt(1 - U*U - V*V); //

        // We want particles traveling in +Z direction so +W for -E, vice versa
        G4int sgn = -1*((0.0<E) - (E<0.0));

        temp.X = X; temp.Y = Y; temp.Z = Z;
        temp.U = U; temp.V = V; temp.W = sgn*W;
        temp.E = fabs(E);
        temp.t = pType, temp.wt = 0.0;
        pList.push_back(temp);

        // G4cout << "(t, E, X, Y, Z, U, V, W) = (" << G4int(pType) << ", " << temp.E << ", " << temp.X << ", " << temp.Y << ", " << temp.Z << ", " << temp.U << ", " << temp.V << ", " << temp.W << ")" << G4endl;
    }
    fPlaceholder += lineSize*batchSize;
    infile.close();

    // move to next phase space file?
    if (fPlaceholder >= flen*lineSize){
        if (debugpsf)
            G4cout << "PSF: End of phase space file reached." << G4endl;
        // begin reading next phase space file
        psfvects.psFiles(threadid).pop_back();
        fPlaceholder = 0;
    }
}
#endif

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
