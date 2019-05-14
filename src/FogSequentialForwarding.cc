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

#include "FogLoadBalancer.h"
#include "FogSequentialForwarding.h"
#include "FogJob_m.h"

namespace fog {


    FogSequentialForwarding::FogSequentialForwarding() {
        // TODO Auto-generated constructor stub

    }

    FogSequentialForwarding::~FogSequentialForwarding() {
        // TODO Auto-generated destructor stub
    }

    bool FogSequentialForwarding::decideProcessLocally(FogJob *job) {
        int pu = FogLoadBalancer::localLoad->getLeastLoadPU();
        int thr = par("THR");
        return (FogLoadBalancer::localLoad->load.at(pu) < thr);
    }

    bool FogSequentialForwarding::decideStartProbes(FogJob *job) {
        return false;
    }

    int FogSequentialForwarding::getFanout() {
        return 0;
    }

    int FogSequentialForwarding::selectNeighbor(std::vector<int> neighbors) {
        std::random_shuffle(FogLoadBalancer::probeGates.begin(), FogLoadBalancer::probeGates.end());
        return (FogLoadBalancer::probeGates.at(0));
    }

    bool FogSequentialForwarding::decideForwardNow() {
        return false;
    }

}
