
#include "Run.hh"

#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4Event.hh"
#include "G4String.hh"

#include <fstream>
#include <sstream>
#include <string>

Run::Run() {
	G4SDManager *sdm = G4SDManager::GetSDMpointer();
	G4MultiFunctionalDetector *mfd = static_cast<G4MultiFunctionalDetector*>(sdm->FindSensitiveDetector(mfd_name));

	if (!mfd) { return; }

    // single beamlets - read spec from file
    std::vector<iTwoVector> beamlet_specs;
    std::ifstream infile("tracked_beamlets.txt");
    if (infile.good()) {
        std::string line;
        std::getline(infile, line);
        std::stringstream ss(line);
        int fx, fy;
        if (ss >> fx >> fy) {
            fmap_size = iTwoVector{fx, fy};
        }

        std::getline(infile, line);
        ss.clear();
        ss.str(line);
        double bsx, bsy;
        if (ss >> bsx >> bsy) {
            beamlet_size = dTwoVector{bsx*mm, bsy*mm};
        }

        std::getline(infile, line);
        ss.clear();
        ss.str(line);
        double px, py, pz;
        if(ss >> px >> py >> pz) {
            fmap_center_pos = G4ThreeVector(px*mm, py*mm, pz*mm);
        }

        // skip line
        infile.ignore(9999, '\n');

        // read beamlet indices
        while (std::getline(infile, line)) {
            ss.clear();
            ss.str(line);
            int bx, by;
            if (ss >> bx >> by) {
                beamlet_specs.push_back(iTwoVector{bx, by});
            }
        }

        infile.close();
    }

    for (const auto& blt : beamlet_specs) {
        tracked_beamlets[blt] = t_hitscoll();
    }

    for (G4int icol=0; icol < mfd->GetNumberOfPrimitives(); ++icol) {
        G4VPrimitiveScorer* scorer = mfd->GetPrimitive(icol);
        G4String full_name = mfd_name + "/" + scorer->GetName();
        G4int collectionID = sdm->GetCollectionID(full_name);
        if (collectionID < 0) {
            G4cerr << "Collection not found." << std::endl;
            continue;
        }
        //referencing a map by [] is an overloaded operator for implicitly inserting values if that element doesnt exist -.- sneaky bastards
        hitsmaps_by_name[full_name] = new t_hitsmap(mfd_name, scorer->GetName());

        for (const auto& blt : beamlet_specs) {
            tracked_beamlets[blt][full_name] = new t_hitsmap(mfd_name, scorer->GetName());
        }
    }


}

Run::~Run() {
    // cleanup dynamic allocations - full beam
    for (string_map_iter it=hitsmaps_by_name.begin(); it!=hitsmaps_by_name.end(); ++it) {
        delete it->second;
    }

    // cleanup dynamic allocations - beamlets
    for (const auto& blt : tracked_beamlets) {
        for (string_map_iter it=blt.second.begin(); it!=blt.second.end(); ++it) {
            delete it->second;
        }
    }
}

void Run::RecordEvent(const G4Event* event)
{
	/*From GEant4 Manual:
	Method to be overwritten by the user for recording events in this (thread-local) run. At the end of the implementation,
	G4Run base-class method for must be invoked for recording data members in the base class.
	*/
    // mandatory
	G4Run::RecordEvent(event);


	G4HCofThisEvent* pHCE = event->GetHCofThisEvent();
	if (!pHCE) {
		return;
	}

    auto* sdm = G4SDManager::GetSDMpointer();

    // full volume detection
	for (string_map_iter it = hitsmaps_by_name.begin(); it != hitsmaps_by_name.end(); ++it) {
        int icol = sdm->GetCollectionID(it->first);
		t_hitsmap* event_map = static_cast<t_hitsmap*>(pHCE->GetHC(icol));

		if (event_map) {
            // Add this event to thread_local hitsmap
			*hitsmaps_by_name[it->first] += *event_map;
		}
	}

    // single beamlet detection
    // determine originating beamlet
    iTwoVector blt = GetBeamletNumber(event);

    auto tracked_beamlet = tracked_beamlets.find(blt);

    if (tracked_beamlet != tracked_beamlets.end() && blt == tracked_beamlet->first) {
        // tally dose (hits) into beamlet specific array
        for (string_map_iter it = tracked_beamlet->second.begin(); it != tracked_beamlet->second.end(); ++it) {
            int icol = sdm->GetCollectionID(it->first);
            t_hitsmap* event_map = static_cast<t_hitsmap*>(pHCE->GetHC(icol));

            if (event_map) {
                // Add this event to thread_local hitsmap
                *tracked_beamlet->second[it->first] += *event_map;
            }
        }
    }

}


void Run::Merge(const G4Run* thread_local_run) {
    /* Called from Global Run object in MultiThreaded mode with each thread_local run object as input
     * The user is responsible for taking results from each thread_local_run object and accumulating
     * them into "*this" which belongs to the global Run object
     */
	const Run *local_run = static_cast<const Run*>(thread_local_run);

    // full beam
	for (string_map_iter it=local_run->hitsmaps_by_name.begin(); it != local_run->hitsmaps_by_name.end(); ++it) {
		G4cout << "Merging HitsMap from Run (full beam): " << it->first << G4endl;
		*hitsmaps_by_name[it->first] +=  *it->second;
	}

    // single beamlet
    for (const auto& tracked_beamlet : local_run->tracked_beamlets) {
        for (string_map_iter it=tracked_beamlet.second.begin(); it != tracked_beamlet.second.end(); ++it) {
            G4cout << "Merging HitsMap from Run (beamlet (" << tracked_beamlet.first.x << "," << tracked_beamlet.first.y << ")): " << it->first << G4endl;
            *tracked_beamlets[tracked_beamlet.first][it->first] += *it->second;
        }
    }

    // mandatory
	G4Run::Merge(thread_local_run);
}

iTwoVector Run::GetBeamletNumber(const G4Event* event) {
    /* Only valid for AP beam with fluence map orthogonal to z-axis */
    G4double x0, y0, z0;
    x0 = event->GetPrimaryVertex()->GetX0();
    y0 = event->GetPrimaryVertex()->GetY0();
    z0 = event->GetPrimaryVertex()->GetZ0();

    int bx = (fmap_size.x-1) - floor(alpha*(x0 - fmap_center_pos.getX())/beamlet_size.x + (fmap_size.x/2.0));
    int by = (fmap_size.y-1) - floor(alpha*(y0 - alpha*fmap_center_pos.getY())/beamlet_size.y + (fmap_size.y/2.0));

    // G4cout << "Event originated from beamlet [" << bx << ", " << by << "]; coords (" <<
    //           x0 << ", " << y0 << ", " << z0 << ")" << G4endl;

    return iTwoVector{bx, by};
}
