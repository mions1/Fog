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

#include "FogParallelProbing.h"

namespace fog {

    Define_Module(FogParallelProbing);

    int k = 3;

    FogParallelProbing::FogParallelProbing() {
        // TODO Auto-generated constructor stub

    }

    FogParallelProbing::~FogParallelProbing() {
        // TODO Auto-generated destructor stub
    }

    bool FogParallelProbing::decideProcessLocally(FogJob *job) {
        ProbeQueue* pq = ProbeQueue::getQueueFromJobId(job->getId(), probesQueue);
        if (pq == NULL)
            return false;

        int leastRemoteLoad = pq->getLeastLoad();
        int leastLocalLoad = localLoad->getLeastLoad();
        return (leastRemoteLoad > leastLocalLoad);
        }

    bool FogParallelProbing::decideStartProbes(FogJob *job) {
        return true;
    }

    int FogParallelProbing::getFanout() {
        return k;
    }

    bool FogParallelProbing::decideForwardNow(FogJob *job, int queryId) {
        return (probesQueue[queryId]->getLeastLoad() == 0);
    }


} /* namespace fog */
