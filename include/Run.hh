#ifndef RUN_HH
#define RUN_HH

#include <map>

#include "G4THitsMap.hh"
#include "G4Run.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class G4Event;
class G4MultiFunctionalDetector;

struct iTwoVector {
    int x, y;
    bool operator==(const iTwoVector& b) const { return (x == b.x && y == b.y); }
    bool operator<(const iTwoVector& b) const { return y < b.y || (y == b.y && x < b.x); }
};
struct dTwoVector {
    double x, y;
};

typedef G4THitsMap<G4double> t_hitsmap;
typedef std::map<G4String, t_hitsmap*> t_hitscoll;
typedef std::map<iTwoVector, t_hitscoll> t_beamlet_colls;
typedef t_hitscoll::const_iterator string_map_iter;

/* User custom Run class that is created by each threadworker after a global run of the same type is started by the G4MTRunManager
 * The RecordEvent() function is performed by each threadworker after each event is concluded - is responsible for processing/saving
 *   any results that have been collected by sensitive volumes into sensitive detector specific "G4THitsMap" containers
 * The Merge() function is called only from the global run object and accepts a single input which is the thread_local run object
 *   from each threadworker.
 */
class Run : public G4Run
{
    public:
        Run();
        virtual ~Run();

        void RecordEvent(const G4Event* event);
        void Merge(const G4Run* thread_local_Run);

        // full volume
		t_hitscoll hitsmaps_by_name;

        // single beamlets
        t_beamlet_colls tracked_beamlets;

    protected:
        G4String mfd_name = "mfd";
        double alpha = 10; // focused GPS magnification factor (DfF/Dsf)
        iTwoVector fmap_size{-1, -1};
        dTwoVector beamlet_size{-1, -1};
        G4ThreeVector fmap_center_pos{0, 0, 0};

        iTwoVector GetBeamletNumber(const G4Event*);
};

#endif
