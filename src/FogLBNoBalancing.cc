//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "FogLBNoBalancing.h"

#include <algorithm>
#include <omnetpp.h>
#include "../../queueinglib/QueueingDefs.h"
#include <omnetpp/cmsgpar.h>
#include <omnetpp/cqueue.h>
#include "FogLoadBalancer.h"
#include "FogJob_m.h"
#include "LoadUpdate_m.h"

/**
 * Implements a non-load balancing policy
 */
namespace fog {

    Define_Module(FogLBNoBalancing);

    FogLBNoBalancing::FogLBNoBalancing() {
        // TODO Auto-generated constructor stub

    }

    FogLBNoBalancing::~FogLBNoBalancing() {
        // TODO Auto-generated destructor stub
    }

    /**
     * Will always process locally beacuse no load balancing
     */
    bool FogLBNoBalancing::decideProcessLocally(FogJob *job) {
        return true;
    }

    bool FogLBNoBalancing::decideStartProbes(FogJob *job) {
        return false;
    }

} /* namespace fog */
