#ifndef RunAction_h
#define RunAction_h 1


#include "G4UserRunAction.hh"
#include "G4String.hh"

#include "Run.hh"

struct iThreeVector {
    int x, y, z;
    int size() { return x*y*z; }
};

class RunAction : public G4UserRunAction
{
    public:
        RunAction();
        ~RunAction() {}

        virtual void BeginOfRunAction(const G4Run *run);
        virtual void EndOfRunAction(const G4Run *run);
        virtual Run* GenerateRun();

    protected:
        void UpdateOutput(const G4MultiFunctionalDetector* mfd, const std::map<G4String, G4THitsMap<G4double>*>&, G4String fsuffix="");

    private:
        G4String mfd_name = "mfd";
        G4int fRTally = 0;
        iThreeVector det_size{-1,-1,-1}; // read from file on construction
};
#endif
