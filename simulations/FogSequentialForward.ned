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

package fog.simulations;

import ned.DelayChannel;
import fog.FogNodeSequentialForward;
import fog.FogSink;
import fog.FogSource;



//
// Sample network, consisting of a source and a sink.
//
network FogSequentialForward
{
    parameters:
        int nSources = default(2);
        double proctimeMul=default(1.0);
        double rho = default(0.8);
        source[*].suggestedTime=default(normal(1.67s * proctimeMul, 0.036s * proctimeMul));
        source[*].sendInterval=default(exponential((1s / rho) * (proctimeMul / 1.67 )));
        fog[*].nServers=default(1);
        fog[*].server[*].serviceTime = -1s;
    submodules:
        source[nSources]: FogSource;
        // FIXME: should be a component that supports only local dispatching
        fog[nSources]: FogNodeSequentialForward;
        sink: FogSink;
    connections:
        for i=0..nSources-1 {
            source[i].out --> fog[i].in++;
            fog[i].out --> sink.in++;
        }
        // add delay
        for i=0..nSources-2, for j=i+1..nSources-1 {
            // FIXME: we should monitor a LAN response time
            fog[i].toNeighbors++ --> {delay = 16.7ms;} -->fog[j].fromNeighbors++;
            fog[j].toNeighbors++ --> {delay = 16.7ms;} --> fog[i].fromNeighbors++;
        }

}
