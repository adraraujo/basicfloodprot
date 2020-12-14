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

#include "inet/mobility/contract/IMobility.h"

#include "NodeInfoSingleton.h"
#include "BandwidthTwoPoints.h"

namespace inet {

Define_Module(BasicFloodProt);

cGlobalRegistrationList nodeinfo_s("nodeinfo_s");

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
        getAllEstdBw();
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
    Packet *packetToSend = queue.front();
    queue.pop();
    L3Address destAddr = Ipv4Address::ALLONES_ADDRESS;
    emit(packetSentSignal, packetToSend);
    socket.sendTo(packetToSend, destAddr, destPort);
    numSent++;
}

void BasicFloodProt::processStart() {
    //Iniciando app
    std::ostringstream str;
    str << packetName << "-" << numSent;
    Packet *packet = new Packet(str.str().c_str());
    const auto &payload = makeShared<PathPayload>();
    int phase = 1;
    std::pair<int,int> flowIdPhase(par("flowId"),phase);

    activeFlows.insert(flowIdPhase);

    Path newPath;
    newPath.push_back(localAddress);
    payload->setPath(newPath);

    payload->setTarget(destAddress);
    payload->setFlowId(par("flowId"));

    payload->setChunkLength(B(par("messageLength")));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    ListBandwidth *newListBw = new ListBandwidth();
    payload->setListbandwith(*newListBw);
    payload->setReqAppOut(par("reqAppOut"));
    payload->setReqAppIn(par("reqAppIn"));
    payload->setPhase(phase); //PhaseInitial=1

    std::pair<int,int> pairFlowIdPhase(par("flowId"),1);
    listFLowIdPhase.push_back(pairFlowIdPhase);

    packet->insertAtBack(payload);

    activeFlows.insert(flowIdPhase);
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

bool BasicFloodProt::isNodeForwarder(Coord A,Coord B,Coord C) {
    double dBC = ((B.x - C.x)*(B.x - C.x)) + ((B.y - C.y)*(B.y - C.y)) + ((B.z - C.z)*(B.z - C.z));
    double dAC = ((A.x - C.x)*(A.x - C.x)) + ((A.y - C.y)*(A.y - C.y)) + ((A.z - C.z)*(A.z - C.z));
    double dAB = ((A.x - B.x)*(A.x - B.x)) + ((A.y - B.y)*(A.y - B.y)) + ((A.z - B.z)*(A.z - B.z));
    if (dBC < dAC && dAB < dAC)
        return true;
    else
        return false;

}

void BasicFloodProt::processTargetNode(Packet *pk){
}


void BasicFloodProt::processForwardNode(Packet *pk){
}


void BasicFloodProt::processPacketRequest(Packet *pk){
}

void BasicFloodProt::processPacketReply(Packet *pk){
}

void BasicFloodProt::processPacketReserve(Packet *pk){
}

void BasicFloodProt::processPacket(Packet *pk) {
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;

    const auto &payload2 = pk->removeAtBack();
    pk->insertAtBack(payload2);
    const Ptr<PathPayload> &result = dynamicPtrCast<PathPayload>(payload2);
    L3Address target = result->getTarget();
    int flowId = result->getFlowId();
    int phase = result->getPhase();
    std::pair<int,int> flowIdPhase(flowId,phase);

    Path currentPath = result->getPath();
    L3Address lastNeighbourNode = currentPath.back();

    L3Address sourceAddress = getAddress(par("sourceAddress"));

    //ListBandwidth listbandwith;
    double reqAppOut = result->getReqAppOut();
    double reqAppIn = result->getReqAppIn();


    numReceived++;
    if (localAddress != target) { //Nao eh destino
        /*        if(localAddress == sourceAddress) {
            std::cout << "Node de Origem: " << localAddress << endl;
        }
         */
        if(activeFlows.find(flowIdPhase) == activeFlows.end()) {
            // Verifica se se estah no caminho,
            Coord lastNeighbourNodeCoord; //ponto A
            Coord localNodeCoord = getMyPosition(); // ponto B
            Coord targetNodeCoord; // ponto C
            cRegistrationList *allNodesInfoList = nodeinfo_s.getInstance();
            for (int i=0; i<allNodesInfoList->size(); i++) {
                NodeInfoSingleton *nodeInfo = static_cast<NodeInfoSingleton *>(allNodesInfoList->get(i));
                if (nodeInfo->getIpaddr() == lastNeighbourNode) {
                    lastNeighbourNodeCoord = nodeInfo->getPosition(); //A
                    //std::cout << "host A: " << lastNeighbourNodeCoord << endl;
                }
                if (nodeInfo->getIpaddr() == target) {
                    targetNodeCoord = nodeInfo->getPosition(); //C
                    //std::cout << "host C: " << targetCoord << endl;
                }

            }
            bool isForwader = isNodeForwarder(lastNeighbourNodeCoord,localNodeCoord,targetNodeCoord);
            if (isForwader) {
                std::ostringstream str;
                str << packetName << "-" << numSent;
                Packet *packet = new Packet(str.str().c_str());
                const auto &payload = makeShared<PathPayload>();

                Path newPath(currentPath);
                newPath.push_back(localAddress);
                payload->setPath(newPath);
                payload->setTarget(target);
                payload->setFlowId(flowId);
                //payload->setListbandwith(listbandwith);
                payload->setReqAppOut(reqAppOut);
                payload->setReqAppIn(reqAppIn);
                payload->setPhase(phase);

                payload->setChunkLength(B(par("messageLength")));
                payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

                packet->insertAtBack(payload);
                queue.push(packet); // insere na fila de envios
                if(!selfMsg->isScheduled()) { //Se nao ha nada previsto para envio, envia
                    selfMsg->setKind(SEND);
                    simtime_t d = simTime() + par("sendInterval");
                    scheduleAt(d, selfMsg);
                }
            }
            activeFlows.insert(flowIdPhase);
        }
    }
    else { //Se for destino


        std::pair<int,int> flowIdSent(flowId,phase);
        if (flowIdSents.find(flowIdSent) == flowIdSents.end()){

            std::cout <<"Banda: "<<currentBw<<endl;

            if(activeFlows.find(flowIdPhase) == activeFlows.end()) {
                for (auto const& it : allEstdInfoList) { // recupera a banda atual
                    if (it->getLastHost() == lastNeighbourNode && it->getCurrentHost() == localAddress) {
                        currentBw = it->getBw();
                    }
                }
            }
            else {
                for (auto const& it : allEstdInfoList) { // recupera a banda atual
                    if (it->getLastHost() == lastNeighbourNode && it->getCurrentHost() == localAddress) {
                        currentBw = currentBw + it->getBw();
                    }
                }
            }
            activeFlows.insert(flowIdPhase);



             // Se fase 1 Out se fase 2 In
            if (result->getPhase() == 1) {
                reqApp = result->getReqAppOut();
            }
            else if (result->getPhase() == 2)
                reqApp = result->getReqAppIn();

            std::cout <<"ReqAppOut: "<<result->getReqAppOut()<<"| ReqAppIn: "<<result->getReqAppIn()<<endl;
            std::cout <<"Phase: "<< phase << "| Banda Requerida: " << reqApp << "| Banda atual: " << currentBw << "| Host: " << localAddress << endl;

            if (currentBw >= reqApp && localAddress != sourceAddress) {
                ///envia packet de volta
                std::cout << "Caminho de ida Percorrido: " << endl; // #inicio Imprime caminho percorrido
                std::for_each(currentPath.begin(), currentPath.end(), [](L3Address &addr) {
                    std::cout << addr << " -> ";
                }); std::cout << localAddress << endl;
                std::cout << "***!Destino alcan�ado***" << endl;// #fim Imprime caminho percorrido


                std::cout << "**Banda de ida: " << currentBw << " **" << endl;
                std::cout << "*****Preparando a volta*****" << endl;
                std::cout << "Nova Origem > " << localAddress << endl;
                std::cout << "Novo Destino > " << sourceAddress << endl;

                std::ostringstream str;
                str << packetName << "-" << numSent;
                Packet *packet = new Packet(str.str().c_str());
                const auto &payload = makeShared<PathPayload>();

                Path newPath;
                newPath.push_back(localAddress);
                payload->setPath(newPath);

                payload->setTarget(sourceAddress);
                payload->setFlowId(flowId);
                //payload->setListbandwith(listbandwith);
                payload->setReqAppOut(reqAppOut);
                payload->setReqAppIn(reqAppIn);
                payload->setPhase(++phase);

                payload->setChunkLength(B(par("messageLength")));
                payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

                packet->insertAtBack(payload);
                queue.push(packet); // insere na fila de envios
                if(!selfMsg->isScheduled()) { //Se nao ha nada previsto para envio, envia
                    selfMsg->setKind(SEND);
                    simtime_t d = simTime() + par("sendInterval");
                    scheduleAt(d, selfMsg);
                }

                flowIdSents.insert(flowIdSent);
            }
            else {
                if (currentBw >= reqApp && localAddress == sourceAddress) {
                    std::cout << "Reserva realizada com sucesso" << endl;
                    flowIdSents.insert(flowIdSent);
                    std::cout << "Caminho de volta Percorrido: " << endl; // #inicio Imprime caminho percorrido
                    std::for_each(currentPath.begin(), currentPath.end(), [](L3Address &addr) {
                        std::cout << addr << " -> ";
                    }); std::cout << localAddress << endl;
                }
            }
        }
    }
    delete pk;
} // #fim processPacket




void BasicFloodProt::handleStartOperation(LifecycleOperation *operation) {
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        L3Address addr = interfaceTable->getInterface(i)->getIpv4Address();
        L3Address loopback = Ipv4Address::LOOPBACK_ADDRESS;
        if (addr != loopback) {
            localAddress = addr;
            //Armazena as informacoes do node em variavel
            NodeInfoSingleton *ownNodeInfo = new NodeInfoSingleton();
            ownNodeInfo->setIpaddr(localAddress);
            ownNodeInfo->setPosition(getMyPosition());
            //Adiciona o node em uma lista global
            cRegistrationList *allNodesInfoList = nodeinfo_s.getInstance();
            allNodesInfoList->add(ownNodeInfo);
            if (par("enableSend")){
                std::cout << "Origem: > " << localAddress << endl;
            }
        }

    }
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(
            *localAddress ?
                    L3AddressResolver().resolve(localAddress) : L3Address(),
                    localPort);
    setSocketOptions();
    if (par("enableSend")) {

        /*L3Address result;
        const char *dst = par("destAddress");
        L3AddressResolver().tryResolve(dst, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << dst << endl;
        destAddress = result;*/

        destAddress = getAddress(par("destAddress"));
        std::cout << "Destino > " << destAddress << endl;
        simtime_t start = std::max(startTime, simTime());
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
        //host[1]
        //par("destAddress").setStringValue("");
        //par("enableSend").setBoolValue(false);
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

Coord BasicFloodProt::getMyPosition() const {
    cModule *node = getContainingNode(this);
    IMobility *mobilityModule = check_and_cast<IMobility*>(node->getSubmodule("mobility"));
    Coord positionNode = mobilityModule->getCurrentPosition();
    return positionNode;
}

void BasicFloodProt::getAllEstdBw() {
    //Cria a lista de hosts com a largura de banda entre eles.
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.2"),Ipv4Address("10.0.0.4"),30.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.2"),Ipv4Address("10.0.0.5"),10.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.2"),Ipv4Address("10.0.0.6"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.2"),Ipv4Address("10.0.0.7"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.2"),Ipv4Address("10.0.0.8"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.2"),Ipv4Address("10.0.0.9"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.2"),Ipv4Address("10.0.0.10"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.4"),Ipv4Address("10.0.0.1"),20.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.5"),Ipv4Address("10.0.0.1"),40.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.6"),Ipv4Address("10.0.0.1"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.7"),Ipv4Address("10.0.0.1"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.8"),Ipv4Address("10.0.0.1"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.9"),Ipv4Address("10.0.0.1"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.10"),Ipv4Address("10.0.0.1"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.3"),Ipv4Address("10.0.0.1"),10.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.5"),Ipv4Address("10.0.0.3"),10.0);
    allEstdInfoList.push_back(estdTwoPoints);
    //volta
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.4"),Ipv4Address("10.0.0.2"),30.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.1"),Ipv4Address("10.0.0.1"),20.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.1"),Ipv4Address("10.0.0.5"),40.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.5"),Ipv4Address("10.0.0.2"),10.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.1"),Ipv4Address("10.0.0.3"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
    estdTwoPoints = new BandwidthTwoPoints(Ipv4Address("10.0.0.3"),Ipv4Address("10.0.0.5"),0.0);
    allEstdInfoList.push_back(estdTwoPoints);
}


L3Address BasicFloodProt::getAddress(const char *name){
    L3Address addr;
    L3AddressResolver().tryResolve(name, addr);
    if (addr.isUnspecified())
        EV_ERROR << "cannot resolve destination address: " << name << endl;
    return addr;
}


} // namespace inet

