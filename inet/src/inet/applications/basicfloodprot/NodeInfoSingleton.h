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

#ifndef INET_APPLICATIONS_BASICFLOODPROT_NODEINFOSINGLETON_H_
#define INET_APPLICATIONS_BASICFLOODPROT_NODEINFOSINGLETON_H_

#include <omnetpp/cownedobject.h>
#include "inet/networklayer/common/L3Address.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

class NodeInfoSingleton: public omnetpp::cOwnedObject {

    L3Address ipaddr;
    Coord position;
public:
    NodeInfoSingleton(): cOwnedObject(){}
    virtual ~NodeInfoSingleton() {}

    // redefined functions
    virtual const char *getClassName() const {return "NodeInfoSigleton";}

    // new functions
    L3Address getIpaddr() {return ipaddr;};
    void setIpaddr(L3Address ip) {this->ipaddr = ip;};
    Coord getPosition() {return position;}
    void setPosition(Coord p) {this->position = p;}
};

} /* namespace inet */

#endif /* INET_APPLICATIONS_BASICFLOODPROT_NODEINFOSINGLETON_H_ */
