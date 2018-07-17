#ifndef __EVENTACTION_HH__
#define __EVENTACTION_HH__

#include "G4UserEventAction.hh"

class G4Event;

class EventAction : public G4UserEventAction {
    public:
        EventAction() {}
        virtual ~EventAction() {}
        void BeginOfEventAction(const G4Event*);
        void EndOfEventAction(const G4Event*);
};

#endif // __EVENTACTION_HH__
