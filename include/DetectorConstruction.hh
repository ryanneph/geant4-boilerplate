#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include <vector>
#include <list>
#include <map>

class DetectorMessenger;
class G4Material;
class G4NistManager;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
struct matStruct {
	G4double den, frac1, frac2;
	G4int numMat, matID1, matID2;
};


class DetectorConstruction : public G4VUserDetectorConstruction
{
	public:
		DetectorConstruction();
		~DetectorConstruction();

		virtual void ConstructSDandField();
		G4VPhysicalVolume*  Construct();
		static DetectorConstruction* getInstance();
		static DetectorConstruction* instance;
		DetectorMessenger			*dMess;


	private:
		G4int nx, ny, nz;
		G4long nxyz;
		G4double dx, dy, dz;
		G4double px, py, pz;
		G4LogicalVolume *lWorld;

		void ReadPhantom();
		void MapMaterials();
		void CreateMaterial(G4long, G4int);
		void CreatePhantom();
		void SanityCheck();

		G4NistManager* man;
		G4Material *G4Air, *G4Water;

		//Input
		std::vector<matStruct> matty;			//Hold all input for processing

		//Intermediate
		std::list<G4double> densList;			//list of densities
		std::vector<G4double> densVec;			//vector of densities

		//Feed to voxelisation
		std::vector<G4Material*> matVec;		//vector of unique materials
		std::vector<G4int> matMap;		//map voxel number to G4Material vector index (for Geant4 later)

        friend class DetectorMessenger;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
