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
#include "FogJob_m.h"
#include "FogPU.h"

namespace fog {

    const int INTERNAL_JOB = 0;
    const int EXTERNAL_JOB = 1;

    FogLoadBalancer::FogLoadBalancer() {
        // TODO Auto-generated constructor stub

    }

    FogLoadBalancer::~FogLoadBalancer() {
        // TODO Auto-generated destructor stub
    }

    /**
     * Gestisce il processamento del job arrivato
     *
     * Se lo SLA è scaduto e se il job è realtime, viene droppato
     * Altrimenti:
     *      ->Se arriva da fuori e non è possibile far forward(multihop=false) allora lo mando ad una mia pu
     *      ->Se è possibile far forward verifico se è il caso
     */
    void FogLoadBalancer::processJob(FogJob *job) {

        //Droppo il job se necessario
        if (checkIfDropJob(job)) {
            dropAndDelete(job);
            dropJobs++; //update statss
            return;
        }

        //Processo localmente se non è possibile fare altrimenti
        if (getSource(job) == EXTERNAL_JOB && !job->getMultiHop()) {
            FogPU *pu = selectLocalPU();
            send(job, pu);
            //forwardJob(pu, job);
            //updatestats
        }
        else {
            //Decido se processare localmente
            if (decideProcessLocally(job)) {
                FogPU *pu = selectLocalPU();
                send(job, pu);
                //forwardJob(pu, job);
                //updatestats
            }
            //Decido se mandare probes oppure seleziono un vicino
            else {
                if (decideStartProbes(job)) {
                    int fanout = getFanout(job);
                    neighs = getNeighbors(job, fanout);
                    int query_id = getQueryId();
                    recordQuery(query_id, job, neighs);
                    sendProbe(neighs, id);
                    //updadestats
                }
                else {
                    neigh=selectNeighbor(j, all_neighs);
                    send(j, neigh);
                    //updatestats;

                }
            }
        }
    }

    void FogLoadBalancer::processProbeAnswer(Answer *a) {

    }

    FogPU* FogLoadBalancer::selectLocalPU() {

    }

    int FogLoadBalancer::getSource(FogJob *job) {
        cGate *gate = job->getArrivalGate();

        if (strcmp(gate->getName(), "outerLoad") == 0)
            return EXTERNAL_JOB;
        return INTERNAL_JOB;
    }

    /**
     * Restituisce vero se il job è da droppare
     * Il job è da droppare se è flaggato come realTime e se è scaduto il SLA
     */
    bool checkIfDropJob(FogJob *job) {
        //simtime_t now = simtime();
        if (job->getRealTime() == 1 && !checkSLA)
            return true;
        return false;
    }

    /**
     * Restituisce vero se lo SLA è scaduto
     */
    bool checkSLA(FogJob *job) {
        if ( sla_scaduto )
            return true;
        return false;
    }

}
