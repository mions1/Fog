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

import fog.FogProcessingDelayPerServerTS;
import fog.FogSink;


//
// Sample network, consisting of a source and a sink.
//
network CloudDataCenter
{
    parameters:
        int nApps = default(1);
        // sink parameters
    submodules:
        appProc[nApps]: FogProcessingDelayPerServerTS {
            parameters:
                appId = index;
        }
        sink: FogSink;
    connections:
        for i=0..nApps-1 {
            appProc[i].out --> sink.in++;
        }
}
