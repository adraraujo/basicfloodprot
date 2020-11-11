//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
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

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/applications/basicfloodprot/BasicFloodProt.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "packets_m.h"
#include <iostream>
#include <algorithm>

namespace inet {

Define_Module(BasicFloodProt);

BasicFloodProt::~BasicFloodProt() {
    cancelAndDelete(selfMsg);
}

void BasicFloodProt::printMe() const {
    std::cout << this->getParentModule() << " > " << this << endl;
}

void BasicFloodProt::initialize(int stage) {
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        interfaceTable = getModuleFromPar<IInterfaceTable>(
                par("interfaceTableModule"), this);
        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        packetName = par("packetName");

        selfMsg = new cMessage("sendTimer");
    }
}

void BasicFloodProt::finish() {
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void BasicFloodProt::setSocketOptions() {
    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    socket.setCallback(this);
}

void BasicFloodProt::sendPacket() {
    printMe();
    std::cout << "BasicFloodProt::sendPacket()" << endl;
    Packet *packetToSend = queue.front();
    queue.pop();

    L3Address destAddr = Ipv4Address::ALLONES_ADDRESS;
    emit(packetSentSignal, packetToSend);
    socket.sendTo(packetToSend, destAddr, destPort);
    numSent++;


}

void BasicFloodProt::processStart() {
    printMe();
    std::cout << "BasicFloodProt::processStart()" << endl;

    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    const auto &payload = makeShared<PathPayload>();

    activeFlows.insert(par("flowId"));
    payload->setTarget(destAddress);
    payload->setFlowId(par("flowId"));
    payload->setChunkLength(B(par("messageLength")));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    Path newPath;
    newPath.push_back(localAddress);
    payload->setPath(newPath);
    packet->insertAtBack(payload);
    activeFlows.insert(par("flowId"));

    queue.push(packet); // insere na fila de envios

    selfMsg->setKind(SEND);
    simtime_t d = simTime() + par("sendInterval");
    scheduleAt(d, selfMsg);
}

void BasicFloodProt::processSend() {
    sendPacket();
    if(!queue.empty()) {
        simtime_t d = simTime() + par("sendInterval");
        selfMsg->setKind(SEND);
        scheduleAt(d, selfMsg);
    }
}

void BasicFloodProt::processStop() {
    socket.close();
}

void BasicFloodProt::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
        case START:
            processStart();
            break;

        case SEND:
            processSend();
            break;
        default:
            throw cRuntimeError("Invalid kind %d in self message",
                    (int) selfMsg->getKind());
        }
    } else
        socket.processMessage(msg);
}

void BasicFloodProt::socketDataArrived(UdpSocket *socket, Packet *packet) {
    // process incoming packet
    processPacket(packet);
}

void BasicFloodProt::socketErrorArrived(UdpSocket *socket,
        Indication *indication) {
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void BasicFloodProt::socketClosed(UdpSocket *socket) {
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void BasicFloodProt::refreshDisplay() const {
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void BasicFloodProt::processPacket(Packet *pk) {
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk)
                   << endl;

    const auto &payload2 = pk->removeAtBack();
    const Ptr<PathPayload> &result = dynamicPtrCast<PathPayload>(payload2);
    int flowId = result->getFlowId();
    L3Address target = result->getTarget();
    Path currentPath = result->getPath();
    numReceived++;
    if (localAddress == target) {
        std::cout << "!!!!!!!!!!!!Destino alcançado" << endl;
        std::cout << "Caminho Percorrido: " << endl;
        std::for_each(currentPath.begin(), currentPath.end(), [](L3Address &addr) {
            std::cout << addr << " -> ";
        });
        std::cout << localAddress << endl;
    } else {
        if (activeFlows.find(flowId) == activeFlows.end()) {
            activeFlows.insert(flowId);

            Path newPath(currentPath);
            newPath.push_back(localAddress);

            std::ostringstream str;
            str << packetName << "-" << numSent;
            Packet *packet = new Packet(str.str().c_str());
            const auto &payload = makeShared<PathPayload>();

            payload->setTarget(target);
            payload->setFlowId(flowId);
            payload->setPath(newPath);
            payload->setChunkLength(B(par("messageLength")));
            payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
            packet->insertAtBack(payload);

            queue.push(packet); // insere na fila de envios

            if(!selfMsg->isScheduled()) { //Se não há nada previsto para envio, envia
                selfMsg->setKind(SEND);
                simtime_t d = simTime() + par("sendInterval");
                scheduleAt(d, selfMsg);
            }
        }
    }

    delete pk;

}

void BasicFloodProt::handleStartOperation(LifecycleOperation *operation) {
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        L3Address addr = interfaceTable->getInterface(i)->getIpv4Address();
        L3Address loopback = Ipv4Address::LOOPBACK_ADDRESS;
        if (addr != loopback)
            localAddress = addr;
    }


    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(
            *localAddress ?
                    L3AddressResolver().resolve(localAddress) : L3Address(),
            localPort);
    setSocketOptions();

    if (par("enableSend")) {
        L3Address result;
        const char *dst = par("destAddress");
        L3AddressResolver().tryResolve(dst, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << dst << endl;
        destAddress = result;
        printMe();
        std::cout << "Destino > " << destAddress << endl;

        simtime_t start = std::max(startTime, simTime());
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void BasicFloodProt::handleStopOperation(LifecycleOperation *operation) {
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void BasicFloodProt::handleCrashOperation(LifecycleOperation *operation) {
    cancelEvent(selfMsg);
    socket.destroy(); //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

L3Address BasicFloodProt::getMyNetAddr() const {

    return L3Address();
}

} // namespace inet

