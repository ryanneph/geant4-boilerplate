#include "EventAction.hh"

#include "G4Event.hh"

void EventAction::BeginOfEventAction(const G4Event* event) {
    // perform actions before the primary tracks begin tracking
    // G4Event contains the list of primary vertices and particles

}

void EventAction::EndOfEventAction(const G4Event* event) {
    // Perform actions after event has completed (all tracks associated with the primary particle have left the event's stack)
    // The G4Event input has a list of primary vertices and particles and collections of hits and trajectories

}

