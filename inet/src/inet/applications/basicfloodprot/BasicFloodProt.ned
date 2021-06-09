// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.basicfloodprot;

import inet.applications.contract.IApp;

//
// Sends UDP packets to the given IP address at the given interval.
// Compatible with both ~Ipv4 and ~Ipv6.
//
// The sending interval can be a constant or a random value (e.g. exponential(1)).
// If the destAddresses parameter contains more than one address, one
// of them is randomly chosen for each packet. An address may be given in the
// dotted decimal notation, or with the module name. (The L3AddressResolver
// class is used to resolve the address.) To disable the model, set
// destAddresses to "".
//
// Received packets are discarded.
//
// The peer can be a ~UdpSink, another ~UdpBasicApp (it handles received packets
// like ~UdpSink), or a ~UdpEchoApp. When used with ~UdpEchoApp, the rcvdPkLifetime
// statistic will contain the round-trip times.
//
simple BasicFloodProt like IApp
{
    parameters:
        string interfaceTableModule = default("^.interfaceTable");
        int localPort = default(5000);  // local port (-1: use ephemeral port)
        string destAddress = default(""); // list of IP addresses, separated by spaces ("": don't send)
        string sourceAddress = default(""); // list of IP addresses, separated by spaces ("": don't send)
        bool enableSend = default(false);
        string localAddress = default("");
        string packetName = default("UdpBasicAppData");
        int destPort = default(5000);
        volatile int messageLength @unit(B) = default(1B); // length of messages to generate, in bytes
        double startTime @unit(s) = default(1s); // application start time (start of the first packet)
        volatile double sendInterval @unit(s) = uniform(0.1s, 0.5s); // should usually be a random value, e.g. exponential(1)
        int flowId = default(-1);
        double reqAppOut  @unit(Kbps) = default(-1.0Kbps);
        double reqAppIn @unit(Kbps) = default(-1.0Kbps);
        bool receiveBroadcast = default(true); // if true, makes the socket receive broadcast packets
        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetReceived)"; record=vector);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkSeqNo](title="received packet sequence number"; source="appPkSeqNo(packetReceived)"; record=vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

