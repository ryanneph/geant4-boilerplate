#include "AllActionInitialization.hh"

#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

#include "G4String.hh"
#include <vector>

AllActionInitialization::AllActionInitialization()
{}

AllActionInitialization::~AllActionInitialization() {}

void AllActionInitialization::BuildForMaster() const
{
	//Optional Action, looks redundant, but necessary for multi-threaded mode.
    SetUserAction(new RunAction());
	//within each run, events, primaries, scoring, etc. are thread-local
	//but each "run" (e.g. /run/beamon 10000) is governed by a master run thread
}

void AllActionInitialization::Build() const
{

	SetUserAction(new PrimaryGeneratorAction);  //Mandatory (page 232 of G4 Manual)

	//Optional Actions
	RunAction *RA = new RunAction();
	SetUserAction(RA);
    SetUserAction(new EventAction());
	SetUserAction(new SteppingAction());
}
