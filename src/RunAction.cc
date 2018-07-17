#include "RunAction.hh"

#include "G4RunManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include "G4THitsMap.hh"

#include "Run.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

// from ../main.cc
extern long int g_eventsProcessed;
extern G4String g_geoFname;

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <unistd.h>

bool file_exists(const std::string& fname) {
    struct stat buf;
    return stat(fname.c_str(), &buf) == 0;
}

RunAction::RunAction()
{
    // remind detector replica size (z is fastest index)
    G4int nx, ny, nz;
    {
        G4String line;
        std::stringstream ss;

        std::ifstream infile;
        infile.open(g_geoFname);

        if (!infile.is_open()){
            G4cerr << "Failed opening Geometry" << G4endl;
            exit(1);
        }
        getline(infile, line);

        ss.str(line);
        // read header
        ss >> nx >> ny >> nz; // nvoxels
        infile.close();
    }
    det_size = {nx, ny, nz};
}

void RunAction::BeginOfRunAction(const G4Run*)
{
	/*
	From Geant4 Manual:
	This method is invoked before entering the event loop. A typical use of this method would be to initialize and/
	or book histograms for a particular run. This method is invoked after the calculation of the physics tables.
	*/

    if(IsMaster()){
        fRTally++;
        return;
    }
}

void RunAction::EndOfRunAction(const G4Run *run)
{
	/*
	From Geant4 Manual:
	This method is invoked at the very end of the run processing. It is typically used for a simple analysis of
	the processed run.
	*/

    if( ! IsMaster()){
		G4cout<<"End of Run - worker thread terminated"<<G4endl;
        return;
    }
	//If we're here, should be master thread, collect all of the worker tallies
    long int nEventsThisRun = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEventToBeProcessed();
    g_eventsProcessed += nEventsThisRun;
    G4cout << nEventsThisRun << " events processed in this run ("<<g_eventsProcessed<<" events in processed so far in the simulation)" << G4endl;
    G4cout << "Updating measurement output files..." << G4endl;

	G4SDManager *sdm = G4SDManager::GetSDMpointer();
	G4MultiFunctionalDetector *mfd =static_cast<G4MultiFunctionalDetector*>(sdm->FindSensitiveDetector(mfd_name));
	if (!mfd) { return; }

    auto *_run = static_cast<const Run*>(run);
    // output full beam
    G4cout << "writing results for \"Full Beam\"" << G4endl;
    UpdateOutput(mfd, _run->hitsmaps_by_name, "");

    // output single beamlet
    for (const auto& hitsmaps : _run->tracked_beamlets) {
    G4cout << "writing results for \"Beamlet ("<<hitsmaps.first.x<<","<<hitsmaps.first.y<<")\"" << G4endl;
        UpdateOutput(mfd, hitsmaps.second, G4String("(") + std::to_string(hitsmaps.first.x) + "," + std::to_string(hitsmaps.first.y) + ")");
    }
}

void RunAction::UpdateOutput(const G4MultiFunctionalDetector* mfd, const std::map<G4String, G4THitsMap<G4double>*>& hitsmaps, G4String fsuffix) {
	//make empty containers to hold our data
	G4double dmax = 0;
	G4double *data = new G4double[det_size.size()];
	memset(data, 0, sizeof(G4double)*det_size.size());

	G4VPrimitiveScorer* scorer;
	G4THitsMap<G4double>* energy_deposits;

	for (G4int ii=0; ii < mfd->GetNumberOfPrimitives(); ++ii) {
		memset(data, 0, sizeof(G4double)*det_size.size());
		dmax = 0;
		scorer = mfd->GetPrimitive(ii);
		energy_deposits = hitsmaps.at(mfd_name + "/" + scorer->GetName());

		for (G4int ix = 0; ix < det_size.x; ++ix) {
			for (G4int iy = 0; iy < det_size.y; ++iy) {
				for (G4int iz = 0; iz < det_size.z; ++iz) {
                    // G4long copyfrom = ix*det_size.y*det_size.z + iy*det_size.z + iz; // XYZ ordering
                    G4long copyto = iz*det_size.y*det_size.x + iy*det_size.x + ix; // ZYX ordering
                    G4long copyfrom = copyto;
					G4double* energy_deposit = (*energy_deposits)[copyfrom];
					G4double value = 0;
					if (energy_deposit) {
						value = (*energy_deposit);
						data[copyto] += value;
					}
					if (value>dmax) {
						dmax = value;
                    }
				}
			}
		}
		G4cout << "Processed hits map for scorer " << "\"mfd/" + scorer->GetName() << "\" with size: (" << det_size.z << ", " << det_size.y << ", " << det_size.x << ") and max: " << dmax << G4endl;

        // read previous checkpoint output, and add to it before writing this checkpoint output
        G4String fname = G4String(scorer->GetName() + fsuffix + ".bin");
        if (file_exists(fname)) {
            std::ifstream infile(fname.c_str(), std::ios::in | std::ios::binary);
            if (infile.fail()) {
                G4cerr << "Error opening dose input file \""<<fname<<"\"" << G4endl;
            } else {
                G4double* tempdata = new G4double[det_size.size()];
                infile.read((char*)tempdata, det_size.size()*sizeof(G4double));

                for (G4long idx=0; idx<det_size.size(); idx++) {
                    data[idx] += tempdata[idx];
                }
                delete[] tempdata;
                infile.close();
            }
        }

		std::ofstream outfile(fname.c_str(), std::ios::out | std::ios::binary);
		if (outfile.fail()) {
			G4cerr << "Error opening dose output file \""<<fname<<"\"" << G4endl;
        } else {
            outfile.write((char*)data, det_size.size() * sizeof(G4double));
            outfile.close();
        }
	}
	delete[] data;

}

Run* RunAction::GenerateRun()
{
	/*
	From GEant4 Manual:
	This method is invoked at the beginning of the BeamOn() method but after confirmation of the conditions
	of the Geant4 kernel. This method should be used to instantiate a user-specific run class object.

	A.k.a. The "run" class is a user-created class for holding data, and performing functions related to information
	we want to collect during the run.  Consider it a data container for what we tally (with functions).
	*/
    return new Run();
}
