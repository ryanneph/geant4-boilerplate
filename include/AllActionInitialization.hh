#ifndef ALLACTIONINITIALIZATION
#define ALLACTIONINITIALIZATION


#include "G4VUserActionInitialization.hh"
#include "G4Types.hh"
#include "G4String.hh"
#include <vector>

class AllActionInitialization : public G4VUserActionInitialization
{
    public:
        AllActionInitialization(std::vector<G4String>);
		AllActionInitialization();
        ~AllActionInitialization();

        void BuildForMaster() const;
        void Build() const;
};

#endif // AllActionInitialization
