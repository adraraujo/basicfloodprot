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

package inet.examples.basicfloodprot;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network BasicFloodProtNet2
{
    parameters:
        int numHosts;
        @display("bgb=650,450");
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=100,300;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        host1: AdhocHost {
            parameters:
                @display("p=171,280;i=device/wifilaptop");
        }
        host2: AdhocHost {
            parameters:
                @display("p=268,143;i=device/wifilaptop");
        }
        host3: AdhocHost {
            parameters:
                @display("p=354,288;i=device/wifilaptop");
        }
        host4: AdhocHost {
            parameters:
                @display("p=354,382;i=device/wifilaptop");
        }
        host5: AdhocHost {
            parameters:
                @display("p=427,200;i=device/wifilaptop");
        }
        host6: AdhocHost {
            parameters:
                @display("p=427,100;i=device/wifilaptop");
        }
        host7: AdhocHost {
            parameters:
                @display("p=560,240;i=device/wifilaptop");
        }


}
