#ifndef NestedParamETERISATION_HH
#define NestedParamETERISATION_HH

#include <vector>

#include "G4Types.hh"
#include "G4ThreeVector.hh"
#include "G4VNestedParameterisation.hh"

class G4VPhysicalVolume;
class G4VTouchable;
class G4VSolid;
class G4Material;

// CSG Entities which may be parameterised/replicated
//
class G4Box;
class G4Tubs;
class G4Trd;
class G4Trap;
class G4Cons;
class G4Sphere;
class G4Orb;
class G4Torus;
class G4Para;
class G4Polycone;
class G4Polyhedra;
class G4Hype;
class vector;

class NestedParam : public G4VNestedParameterisation
{
  public:

    NestedParam(std::vector<G4int>,std::vector<G4Material*>);
    virtual ~NestedParam();

    G4Material* ComputeMaterial(G4VPhysicalVolume*, const G4int copyNoZ, const G4VTouchable* parentTouch);
	G4Material* GetMaterial(G4int i) const;

    G4int       GetNumberOfMaterials() const;

    G4int GetMaterialIndex( G4int nx, G4int ny, G4int nz) const;
    unsigned int GetMaterialIndex( unsigned int copyNo) const;

    void SetNoVoxel( G4int nx, G4int ny, G4int nz );
	void SetDimVoxel( G4double nx, G4double ny, G4double nz );

    void ComputeTransformation(const G4int no, G4VPhysicalVolume *currentPV) const;

    // Additional standard Parameterisation methods,
    // which can be optionally defined, in case solid is used.

    void ComputeDimensions(G4Box &, const G4int, const G4VPhysicalVolume *) const;

    using G4VNestedParameterisation::ComputeMaterial;


  private:

    G4double					dx, dy, dz;
    G4int						nx, ny, nz;
	std::vector<G4int>			matMap; // in ZYX ordering
    std::vector<G4Material*>	matVec;



private:  // Dummy declarations to get rid of warnings ...
	void ComputeDimensions(G4Trd&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Trap&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Cons&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Sphere&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Orb&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Torus&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Para&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Hype&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Tubs&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Polycone&, const G4int, const G4VPhysicalVolume*) const {}
	void ComputeDimensions(G4Polyhedra&, const G4int, const G4VPhysicalVolume*) const {}

};
#endif
