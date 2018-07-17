#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

#include <sstream>

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithABool.hh"
#include "G4RunManager.hh"

extern G4String g_geoFname;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::DetectorMessenger(DetectorConstruction * Det)
:Detector(Det)
{
  Dir = new G4UIdirectory("/det/");
  Dir->SetGuidance(" Detector control.");

  // geoCmd = new G4UIcmdWithAString("/det/geo", this);
  // geoCmd->SetGuidance("Set geometry file used in building the detector.");
  // geoCmd->SetParameterName("geoFile", true);
  // geoCmd->SetDefaultValue("geo.txt");

  // geoshowCmd = new G4UIcmdWithoutParameter("/det/show",this);
  // geoshowCmd->SetGuidance("List geometry details.");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::~DetectorMessenger()
{
	delete   Dir;
    delete   geoCmd;
    delete   geoshowCmd;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command,G4String newValue) {
    // if (command == geoCmd) {
    //     g_geoFname = newValue;
    // } else if (command == geoshowCmd) {
    //     G4cout << "Geometry file in use is: \"" << g_geoFname << "\""  << G4endl;
    // }
}
G4String DetectorMessenger::GetCurrentValue(G4UIcommand* command) {
    if (command == geoCmd) {
        return g_geoFname;
    }
    return G4String("");
}
