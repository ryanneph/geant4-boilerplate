#include "SteppingAction.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4StepPoint.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// User specified actions to perform at every step (everytime an event is updated)
void SteppingAction::UserSteppingAction(const G4Step* step)
{
	/*
	G4cout << step->GetTrack()->GetParentID() << G4endl;
	G4cout << step->GetTotalEnergyDeposit() << G4endl;
	G4cout << step->GetStepLength() << G4endl;
	G4cout << step->GetTrack()->GetVolume()->GetName() << G4endl;
	G4cout << step->GetTrack()->GetVertexKineticEnergy() << G4endl;
	*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

