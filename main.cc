#ifdef G4MULTITHREADED
    #include "G4MTRunManager.hh"
    #include "G4Threading.hh"
	#include <unistd.h>
#else
    #include "G4RunManager.hh"
#endif
#include "Randomize.hh"
#include "G4UImanager.hh"    // enables use of .in files
#include "G4VisExecutive.hh" // enables heprap file output
#include "G4UIExecutive.hh"  // enables input files (UI Commands)

#include "AllActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"              // required
#include "PrimaryGeneratorAction.hh" // just to get NUM_THREADS definition
#include "RunAction.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4ios.hh"

#include <vector>
#include <exception.h>

// keep a count of the number of events that have already been procesed; updated after each run by the master thread
long int g_eventsProcessed = 0;
G4String g_geoFname; // set using argv[1]

int main( int argc, char** argv )
{
    // Parse command line args
    if (argc > 2) {
        g_geoFname = argv[1];
        G4cout << "Using geometry file: \""<<g_geoFname<<"\""<<G4endl;
    } else {
        G4cout << "Usage: " << argv[0] << " <geometry-file> <input-file> [<input-file> ...]" << G4endl;
        exit(1);
    }

	G4cout << "Creating Run Manager ..." << G4endl;
    #ifdef G4MULTITHREADED
        G4cout << "Running multithreaded." << G4endl;
        G4MTRunManager *runManager = new G4MTRunManager{};

        // enable run-level seeding (instead of event-level seeding) in MT;
        // recommended for high event-count runs (like all medical physics dose calculation)
        runManager->SetSeedOncePerCommunication(1);
        G4cout << "Using MT seeding strategy: ";
        switch (runManager->SeedOncePerCommunication()) {
            case 0 :
                G4cout << "event-level" << G4endl
                    << " (warning: event-level seeding is unsuitable for high-run-count simulation, prefer run-level instead)";
                break;
            case 1 :
                G4cout << "run-level";
                break;
            default :
                G4cout << runManager->SeedOncePerCommunication();
        }
        G4cout << G4endl;
        #ifdef USEPHASESPACE
            // set NUM_THREADS in PrimaryGeneratorAction.hh
            G4int number_of_cores = G4Threading::G4GetNumberOfCores();
            number_of_cores = NUM_THREADS;
            G4cout << "Number of cores/threads in use: "
                << number_of_cores << " of "
                << G4Threading::G4GetNumberOfCores()
                << G4endl;
            runManager->SetNumberOfThreads(number_of_cores);
        #endif
    #else
        G4cout << "Running single threaded." << G4endl;
		G4RunManager* runManager = new G4RunManager;
    #endif

    // prng seed
    G4int t1 = time(NULL);
    G4int seed = t1%900000000; // prevents seed overflow
    // G4Random::setTheEngine(new CLHEP::Ranlux64Engine());
    // G4Random::setTheEngine(new CLHEP::MTwistEngine()); // uses two seeds
    auto *engine = G4Random::getTheEngine();
    G4Random::setTheSeed(seed);
    G4cout << "Psuedo-RNG seed: " << seed << G4endl;
    engine->showStatus();


    /*------------------ Mandatory Init Classes ---------------------------------------*/
    // Geometry - construct
    DetectorConstruction* det = DetectorConstruction::getInstance();
    runManager->SetUserInitialization(det);

    // Physics - register instance of selected physics with runManager
    G4VModularPhysicsList* physics = new PhysicsList;
    runManager->SetUserInitialization(physics);

    // Register all "UserActions": Particle generation, Stepping Actions, Event Actions ... etc
    G4VUserActionInitialization* AAI = new AllActionInitialization();
    runManager->SetUserInitialization(AAI);
    /*---------------------------------------------------------------------------------*/

    // Visualization
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // Issue runtime commands to program in the form of input files (*.in)
    G4UImanager* UI = G4UImanager::GetUIpointer();

    // Parse Input file(s)
    // input file must contain "/run/beamOn ###" for run to start
    G4String command = "/control/execute ";
    for (int i=2; i<argc; i++) {
        G4String macroFileName = argv[i];
        UI->ApplyCommand(command+macroFileName);
    }

    G4int t2 = time(NULL);
    G4cout << "Total Runtime: " << difftime(t2, t1) << " seconds" << G4endl;

    // job termination
    delete runManager;
    return 0;
}
