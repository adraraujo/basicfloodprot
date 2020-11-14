/*
 * NodeInfoSigleton.h
 *
 *  Created on: 13 de nov de 2020
 *      Author: Adriano Araújo
 */

#ifndef INET_APPLICATIONS_BASICFLOODPROT_NODEINFOSIGLETON_H_
#define INET_APPLICATIONS_BASICFLOODPROT_NODEINFOSIGLETON_H_

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

extern cGlobalRegistrationList nodeinfo_s;

class cNodeInfoSigleton : public cOwnedObject
{
    L3Address ipaddr;
    Coord position;
public:
    cNodeInfoSigleton() : cOwnedObject(){}
    virtual ~cNodeInfoSigleton() {}

    // redefined functions
    virtual const char *getClassName() const {return "cNodeInfoSigleton";}

    // new functions
    L3Address getIp() {return ipaddr;};
    void setIp(L3Address ip) {this->ipaddr = ip;};
    Coord getPosition() {return position;}
    void setPosition(Coord p) {this->position = p;}

};




}  // namespace inet




#endif /* INET_APPLICATIONS_BASICFLOODPROT_NODEINFOSIGLETON_H_ */
