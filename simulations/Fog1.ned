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

import fog.FogNode;
import fog.FogSink;
import fog.FogSource;



//
// Sample network, consisting of a source and a sink.
//
network Fog1
{
    parameters:
        int nSources = default(2);
        // sink parameters
    submodules:
        source[nSources]: FogSource;
        fog[nSources]: FogNode;
        sink: FogSink;
    connections:
        for i=0..nSources-1 {
            source[i].out --> fog[i].in++;
            fog[i].out --> sink.in++;
        };
        for i=0..nSources-2, for j=i+1..nSources-1 {
            fog[i].toNeighbors++ --> fog[j].fromNeighbors++;
            fog[j].toNeighbors++ --> fog[i].fromNeighbors++;
        }

}
