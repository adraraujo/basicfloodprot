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

#ifndef __INET_BasicFloodProt_H
#define __INET_BasicFloodProt_H \
    EXECUTE_ON_STARTUP(nodesinfosingleton.getInstance()->add(new NodeInfo()))

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include <set>
#include <queue>
#include <list>
#include <cstdlib>

#include "inet/common/geometry/common/Coord.h"
#include "NodeInfoSigleton.h"

namespace inet {

class EstdBandwidth;

/**
 * UDP application. See NED for more info.
 */
class INET_API BasicFloodProt : public ApplicationBase, public UdpSocket::ICallback
{
protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };

    // parameters
    L3Address destAddress;
    const char *destAddrs;
    int localPort = -1, destPort = -1;
    simtime_t startTime;
    simtime_t stopTime;
    const char *packetName = nullptr;
    std::set<int> activeFlows;
    std::queue<Packet*> queue; //Fila de envio de pacotes


    // state
    UdpSocket socket;
    cMessage *selfMsg = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;
    IInterfaceTable *interfaceTable = nullptr;
    L3Address localAddress;
private:


    EstdBandwidth *estdTwoPoints;
    std::list<EstdBandwidth*> allEstdInfoList;



protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override; //1
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    // chooses random destination address
    virtual void sendPacket();
    virtual void processPacket(Packet *msg);
    virtual void setSocketOptions();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override; //2
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;
    virtual L3Address getMyNetAddr() const;
    virtual void printMe() const;
    virtual Coord getMyPosition() const;
    virtual void getAllEstdBw();

public:
    BasicFloodProt() {}
    ~BasicFloodProt();
};


} // namespace inet
#endif // ifndef __INET_BasicFloodProt_H

