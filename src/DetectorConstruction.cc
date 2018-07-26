#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"

#include "G4Element.hh"
#include "G4Material.hh"

//Solid volumes (shapes)
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4Trd.hh"

//Logical and Physical volumes (materials, placement)
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"

//Boolean Solids (union, subtraction, intersection etc.)
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"

//PVReplica for voxelization,
#include "G4PVReplica.hh"

//G4PVParameterised and nestedparam for CT materials
#include "G4PVParameterised.hh"
#include "NestedParam.hh"

//For Scoring
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4PSDoseDeposit3D.hh"
#include "G4SDParticleFilter.hh"
#include "G4PSPassageCellCurrent3D.hh"
#include "G4UserParticleWithDirectionFilter.hh"

//Quality of Life includes
#include "G4NistManager.hh"
#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <exception>

extern G4String g_geoFname;

using namespace std;
DetectorConstruction* DetectorConstruction::instance = 0;
DetectorConstruction* DetectorConstruction::getInstance()
{
	if (instance == 0) instance = new DetectorConstruction();
	return instance;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
DetectorConstruction::DetectorConstruction()
{
	dMess = new DetectorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
	delete dMess;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{

	G4cout << "Entering DetectorConstruction::Construct()" << G4endl;

	G4double wx, wy, wz;
    wx = wy = 30 * cm;
	wz = 410 * cm;

	//declare starting materials first
	//Note that manager and air water ... are global to the class
	man = G4NistManager::Instance();
	G4Air = man->FindOrBuildMaterial("G4_AIR");
    G4Water = man->FindOrBuildMaterial("G4_WATER");

	G4Box *sWorld = new G4Box("world", wx / 2., wy / 2., wz / 2.);
	lWorld = new G4LogicalVolume(sWorld, G4Air, "World");
	G4VPhysicalVolume *pWorld = new G4PVPlacement(0, G4ThreeVector(), lWorld, "World", 0, false, 0);

	//compile and run, visualize
    ReadPhantom();
    MapMaterials();
    CreatePhantom();

    SanityCheck();
	return pWorld;
}

void DetectorConstruction::ReadPhantom() {
	G4String line;
	std::stringstream ss;

	std::ifstream infile;
	infile.open(g_geoFname);

	if (!infile.is_open()){
		G4cerr << "Failed opening Geometry" << G4endl;
		exit(1);
	}

	// ss.str(line);
    // read header
	infile >> nx >> ny >> nz; // nvoxels
    infile >> dx >> dy >> dz; // voxelsize (mm)
    infile >> px >> py >> pz; // position of array center (mm)
	nxyz = nx*ny*nz;
    infile.ignore(1, '\n'); // move to next line

	//sanity check
	G4cout << "Array size: " << nx <<" "<< ny << " " << nz << G4endl <<
              "Voxel size (mm): "<< dx << " " << dy << " " << dz << G4endl <<
              "Center Position (mm): " << px << " " << py << " " << pz << G4endl;

	ss.str("");
	ss.clear();

    // read material specifications
	G4long i = 0;
	while (getline(infile, line)){
		if (line.length() < 1) { //last line is empty
			infile.close();
			break;
		}
		ss.str(line);
		matStruct temp;
		ss >> temp.den >> temp.numMat >> temp.matID1 >> temp.frac1;
		if (temp.numMat == 2) {
			ss >> temp.matID2 >> temp.frac2;
        }

        temp.den *= g/cm3;
		matty.push_back(temp);
		ss.str("");
		ss.clear();

		densList.push_back(temp.den);
		i++;
	}
	if((G4long)matty.size()!=nxyz) { //number of voxel mismatch
		G4cerr << "mismatch between nxyz in header and number of lines" << G4endl;
        throw runtime_error("mismatch between nxyz in header and number of lines in file");
    }
}

void DetectorConstruction::MapMaterials() {

	//List has sort and unique, vector has random access
	//Create, Sort and prune density list to contain only unique densities to prevent multiple material redefinition
	G4cout << "all materials prior sort/unique " << densList.size() << G4endl;
	densList.sort();
	densList.unique();

	//Migrate list to vector for random access
	densVec.resize(densList.size());	//make vector container large enough
	std::copy(densList.begin(), densList.end(), densVec.begin());
	G4cout << "all materials after sort/unique " << densVec.size() << G4endl;

	//initialize material vector to final size of unique materials, all with null material

	for (uint i = 0; i < densVec.size(); i++){
		matVec.push_back(0);
    }

	//Go through all voxels, find corresponding density and map voxel to that unique density
	G4long idx;
	for (uint i = 0; i < matty.size(); i++)
	{
		//look in pruned density list for current voxel's density via:
		for (idx = 0; idx < (G4long)densVec.size(); idx++)
		{
			//match current voxel i's density with a unique density index idx
			if (matty[i].den == densVec[idx]) {
				//assign this mapping to the material
				matMap.push_back(idx);

				//create the material if it doesn't already exist
				CreateMaterial(i, idx);
				break;
			}
		}
	}

}

void DetectorConstruction::CreateMaterial(G4long i, G4int idx) {
	//reminder: i corresponds to specific voxel, idx corresponds to the material we want
	if (matVec[idx] != 0)
		return; //this material is already here, nothing to do.  Put this first so we jump out quick

	//if we're here, then the material doesn't exist, time to create a material

	//local variables easier to work with
    const auto& mat = matty[i];

	std::stringstream ss; //holder for material name
	ss << "mat" << idx;

	if (mat.numMat == 1) { //easy, just making one material
		if (mat.matID1 == 0) //water
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_WATER", mat.den);
		else if (mat.matID1 == 1) //ICRP Lung - deflated
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_LUNG_ICRP", mat.den);
		else if (mat.matID1 == 2) //titanium
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_Ti", mat.den);
		else if (mat.matID1 == 3) //icrp adipose tissue
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_ADIPOSE_TISSUE_ICRP", mat.den);
		else if (mat.matID1 == 4) //muscle
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_MUSCLE_STRIATED_ICRU", mat.den);
		else if (mat.matID1 == 5) //bone
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_BONE_COMPACT_ICRU", mat.den);
		else if (mat.matID1 == 6) //air
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_AIR", mat.den);
		else if (mat.matID1 == 7) //aluminum
			matVec[idx] = man->BuildMaterialWithNewDensity(ss.str(), "G4_Al", mat.den);
        else
            throw runtime_error("material undefined");
    } else { throw runtime_error("multi-material voxels not yet implemented"); }
}

void DetectorConstruction::CreatePhantom() {
	/////////////////From lecture 8/////////////////
	G4double boxx, boxy, boxz;
	boxx = nx*dx;
    boxy = ny*dy;
    boxz = nz*dz;

	G4Box * sBox = new G4Box("sBox", boxx / 2., boxy / 2., boxz / 2.);
	G4LogicalVolume *lBox = new G4LogicalVolume(sBox, G4Water, "lBox");
	new G4PVPlacement(0, G4ThreeVector(px, py, pz), lBox, "pBox", lWorld, false, 0, true);

	G4VSolid *sRepZ = new G4Box("sRepZ", boxx / 2., boxy / 2., dz / 2.);
	G4LogicalVolume *lRepZ = new G4LogicalVolume(sRepZ, G4Air, "lRepZ");
	new G4PVReplica("pRepZ", lRepZ, lBox, kZAxis, nz, dz);

	G4VSolid *sRepY = new G4Box("sRepY", boxx / 2., dy / 2., dz / 2.);
	G4LogicalVolume *lRepY = new G4LogicalVolume(sRepY, G4Air, "lRepY");
	new G4PVReplica("pRepY", lRepY, lRepZ, kYAxis, ny, dy);

	G4VSolid *sRepX = new G4Box("sRepX", dx / 2., dy / 2., dz / 2.);
	G4LogicalVolume *lRepX = new G4LogicalVolume(sRepX, G4Water, "lRepX");

	/////////////////////////Not from Lecture 8///////////

	NestedParam* param = new NestedParam(matMap, matVec);  //defines material mapping that overrides voxel logvol
    param->SetDimVoxel(dx, dy, dz);
    param->SetNoVoxel(nx, ny, nz);

	new G4PVParameterised("ctVox", lRepX, lRepY, kXAxis, nx, param); //a parameterised pvplacement
}

void DetectorConstruction::SanityCheck() {
	std::ofstream outfile;

	outfile.open("InputDensity.bin", std::ios::out | std::ios::binary);
	for (G4long i = 0; i < (G4long)matty.size(); i++) {
        float val = float(matty[i].den)/(g/cm3);
		outfile.write((char*)(&val), sizeof(float));
	}
	outfile.close();
}

void DetectorConstruction::ConstructSDandField() {
///////// GENERATE SENSITIVE DETECTORS AND SCORERS ////////////
    G4SDManager *sdmanager = G4SDManager::GetSDMpointer();

    G4MultiFunctionalDetector *mfd = new G4MultiFunctionalDetector("mfd");
    G4cout << "Attaching Dose MFD of name " << mfd->GetName() << " to SDmanager" << G4endl;
    sdmanager->AddNewDetector(mfd);
    SetSensitiveDetector("lRepX", mfd);

    // create filters
    G4SDParticleFilter* gammaFilter = new G4SDParticleFilter("gammaFilter", "gamma");
    gammaFilter->show();

    // std::vector<G4String> electronpositron = {"e-", "e+"};
    // G4SDParticleFilter* electronFilter = new G4SDParticleFilter("electronFilter", electronpositron);
    // electronFilter->add("e+");
    // electronFilter->show();

    // G4UserParticleWithDirectionFilter* forwardElectronFilter = new G4UserParticleWithDirectionFilter("forwardElectronFilter", electronpositron, G4ThreeVector(0,0,1) );
    // forwardElectronFilter->show();
    // G4UserParticleWithDirectionFilter* backwardElectronFilter = new G4UserParticleWithDirectionFilter("backwardElectronFilter", electronpositron, G4ThreeVector(0,0,-1));
    // backwardElectronFilter->show();


    // total dose
    // don't forget to set depi/j/k to 0,1,2 for ZYX ordering
    G4PSDoseDeposit3D* dose3d = new G4PSDoseDeposit3D("dose3d", nz, ny, nx);
    G4cout << "Attaching primitive scorer of name " << dose3d->GetName() << " to mfd" << G4endl;
    mfd->RegisterPrimitive(dose3d);

    // // forward electron dose
    // G4PSDoseDeposit3D* fedose3d = new G4PSDoseDeposit3D("fedose3d", nx, ny, nz);
    // fedose3d->SetFilter(forwardElectronFilter);
    // G4cout << "Attaching primitive scorer of name " << fedose3d->GetName() << " to mfd" << G4endl;
    // mfd->RegisterPrimitive(fedose3d);

    // // reverse electron dose
    // G4PSDoseDeposit3D* bedose3d = new G4PSDoseDeposit3D("bedose3d", nx, ny, nz);
    // bedose3d->SetFilter(backwardElectronFilter);
    // G4cout << "Attaching primitive scorer of name " << bedose3d->GetName() << " to mfd" << G4endl;
    // mfd->RegisterPrimitive(bedose3d);

    // // electron fluence - counts tracks filtered to electrons
    // G4PSPassageCellCurrent3D* electronFluence3D = new G4PSPassageCellCurrent3D("electronFluence", nx, ny, nz);
    // electronFluence3D->SetFilter(electronFilter);
    // G4cout << "Attaching primitive scorer of name " << bedose3d->GetName() << " to mfd" << G4endl;
    // mfd->RegisterPrimitive(electronFluence3D);

    // photon fluence - counts tracks filtered to gammas
    G4PSPassageCellCurrent3D* photonFluence3D = new G4PSPassageCellCurrent3D("photonFluence", nz, ny, nx);
    photonFluence3D->SetFilter(gammaFilter);
    G4cout << "Attaching primitive scorer of name " << photonFluence3D->GetName() << " to mfd" << G4endl;
    mfd->RegisterPrimitive(photonFluence3D);
}
