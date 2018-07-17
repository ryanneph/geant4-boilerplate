#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

// G4VModularPhysicsList inherits from G4VUserPhysicsList and allows construction with pre-built physics "modules"
class PhysicsList: public G4VModularPhysicsList
{
public:
  PhysicsList();
  virtual ~PhysicsList();

  virtual void SetCuts();
};

#endif
