#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

// USER DEFINITIONS
#define NUM_THREADS 24
#define NUM_PHSP_FILES NUM_THREADS

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include <string>
#include <list>
#include <vector>

/* define to generate events from phasespace files, otherwise use input file GPS specification */
// #define USEPHASESPACE


class G4GeneralParticleSource;
class G4Event;
class G4ParticleGun;
class G4ParticleTable;
class G4PrimaryParticle;

//Custom data container for particle phasespace values
struct pspinfo
{
	G4float X;
	G4float Y;
	G4float Z;
	G4float U;
	G4float V;
	G4float W;
	G4float E;
	G4float wt;
	G4int t;
};
#ifdef USEPHASESPACE
class psfvectlist {
    private:
#ifdef G4MULTITHREADED
        std::vector<std::string> m_psFiles[NUM_THREADS];
#else
        std::vector<std::string> m_psFiles;
#endif // G4MULTITHREADED
    public:
        std::vector<std::string>& psFiles(G4int threadid) {
#ifdef G4MULTITHREADED
            if (threadid < 0 || threadid >= NUM_THREADS) {
                G4cerr << "Error retrieving psFiles vector for threadid " << threadid << G4endl;
            } else {
                return m_psFiles[threadid];
            }
#else
            // threadid should be -2 but we dont need it
            return m_psFiles;
#endif // G4MULTITHREADED
        }
};
#endif // USEPHASESPACE

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
   ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event*);
  private:
    void init();
    void generate(G4Event*);

#ifdef USEPHASESPACE
    G4ParticleGun* fParticleGun;
    void BatchHistories();
    G4int fPlaceholder;

    //Supplemental Code
    std::list<pspinfo> pList;
    psfvectlist psfvects;
    int default_batchSize = 10000;
#else
    G4GeneralParticleSource* fParticleGun;
#endif
    G4ParticleTable* fPT;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
