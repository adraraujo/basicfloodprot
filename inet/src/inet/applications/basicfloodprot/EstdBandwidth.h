/*
 * EstdBandwidth.h
 *
 *  Created on: 12 de nov de 2020
 *      Author: Adriano Araújo
 */

#ifndef INET_ESTDBANDWIDTH_H_
#define INET_ESTDBANDWIDTH_H_

#include "inet/networklayer/common/L3Address.h"

namespace inet {

class EstdBandwidth {
    L3Address ipAddrA;
    L3Address ipAddrB;
    double bandwidth;


public:
    EstdBandwidth(L3Address ipA,L3Address ipB,double bw) {
        this->ipAddrA = ipA;
        this->ipAddrB = ipB;
        this->bandwidth = bw;
    }
    virtual ~EstdBandwidth() {}

    L3Address getIpAddrA() {return this->ipAddrA;}
    L3Address getIpAddrB() {return this->ipAddrB;}
    double getBw(){return this->bandwidth;}

};


}
#endif /* INET_ESTDBANDWIDTH_H_ */
