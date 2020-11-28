/*
 * EstdBwNodesSigleton.h
 *
 *  Created on: 16 de nov de 2020
 *      Author: Adriano Araújo
 */

#ifndef INET_APPLICATIONS_BASICFLOODPROT_ESTDBWNODESSIGLETON_H_
#define INET_APPLICATIONS_BASICFLOODPROT_ESTDBWNODESSIGLETON_H_

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"

namespace inet {

extern cGlobalRegistrationList estdbwnodes_s;

class cEstdBwNodesSigleton : public cOwnedObject
{
    L3Address ipAddrA;
    L3Address ipAddrB;
    double bandwidth;
public:
    cEstdBwNodesSigleton(L3Address ipA,L3Address ipB,double bw) : cOwnedObject(){
        this->ipAddrA = ipA;
        this->ipAddrB = ipB;
        this->bandwidth = bw;
    }
    virtual ~cEstdBwNodesSigleton() {}

    virtual const char *getClassName() const {return "cEstdBwNodesSigleton";}

    // new functions
    double getBw(){return this->bandwidth;}

};

}  // namespace inet



#endif /* INET_APPLICATIONS_BASICFLOODPROT_ESTDBWNODESSIGLETON_H_ */
