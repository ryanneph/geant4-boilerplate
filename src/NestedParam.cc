#include "NestedParam.hh"

#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include <vector>

NestedParam::NestedParam(std::vector<G4int> inMap,std::vector<G4Material*> inVec):
G4VNestedParameterisation(), matVec(inVec), matMap(inMap)
{
}

NestedParam::~NestedParam()
{}

void NestedParam::SetNoVoxel( G4int tempnx, G4int tempny, G4int tempnz ) {
  nx = tempnx;
  ny = tempny;
  nz = tempnz;
}

void NestedParam::SetDimVoxel( G4double tempdx, G4double tempdy, G4double tempdz ) {
  dx = tempdx/2.;
  dy = tempdy/2.;
  dz = tempdz/2.;
}

//
// Material assignment to geometry.
G4Material* NestedParam::ComputeMaterial(G4VPhysicalVolume*, const G4int copyNoX, const G4VTouchable* parentTouch)
{
	// protection for initialization and vis at idle state, just spit back something
	if(parentTouch==0) return matVec[0];

	// Copy number of Z and Y are obtained from replication number.
	// Copy number of X is the copy number of current voxel.

	G4int iz = parentTouch->GetReplicaNumber(1);  //slightly different indexing, since we are already getting parent volume
	G4int iy = parentTouch->GetReplicaNumber(0);  //so depth = depth-1 from previous examples

	G4int ix = copyNoX;

	if(ix<0) //This used to happen, not sure if it still does.
		ix=0;

	G4long copyNo = iz*ny*nx + nx*iy + ix;

	return matVec[matMap[copyNo]];
}

//------------------------------------------------------------------
unsigned int NestedParam::GetMaterialIndex( unsigned int copyNo ) const {
	return copyNo;
}

// Number of Materials
// Material scanner is required for preparing physics tables and so on before
// starting simulation, so that G4 has to know number of materials.

G4int NestedParam::GetNumberOfMaterials() const
{
    return matVec.size();
}

//
// GetMaterial
//  This is needed for material scanner and realizing geometry.  Geant4 will call this at the start.
//
G4Material* NestedParam::GetMaterial(G4int i) const {
  return matVec[i];
}

// Transformation of voxels.
void NestedParam::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume* physVol) const {
  //Just copied from example, for voxelization with X as "fastest" dimension

  G4ThreeVector position((copyNo+1)*dx - dx*nx, 0.,0.);
  position = G4ThreeVector((copyNo)*dx*2 - (nx-1)*dx , 0., 0.);
  physVol->SetTranslation(position);
}

void NestedParam::ComputeDimensions( G4Box& box, const G4int, const G4VPhysicalVolume* ) const {
  box.SetXHalfLength(dx);
  box.SetYHalfLength(dy);
  box.SetZHalfLength(dz);
}
