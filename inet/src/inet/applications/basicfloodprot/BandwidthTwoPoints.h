/*
 * BandwidthTwoPoints.h
 *
 *  Created on: 4 de dez de 2020
 *      Author: Adriano Araújo
 */

#ifndef INET_APPLICATIONS_BASICFLOODPROT_BANDWIDTHTWOPOINTS_H_
#define INET_APPLICATIONS_BASICFLOODPROT_BANDWIDTHTWOPOINTS_H_

#include "inet/networklayer/common/L3Address.h"

namespace inet {

class BandwidthTwoPoints {
    L3Address lastHost;
    L3Address currentHost;
    double bandwidth;


public:
    BandwidthTwoPoints(){}

    BandwidthTwoPoints(L3Address ipA,L3Address ipB,double bw) {
        this->lastHost = ipA;
        this->currentHost = ipB;
        this->bandwidth = bw;
    }
    virtual ~BandwidthTwoPoints() {}

    virtual L3Address getLastHost() {return this->lastHost;}
    virtual L3Address getCurrentHost() {return this->currentHost;}
    virtual double getBw(){return this->bandwidth;}
    //virtual void setLastHost(L3Address h) {return this->lastHost;}
    //virtual void setCurrentHost(L3Address h) {return this->currentHost;}
    //virtual void setBw(){return this->bandwidth;}

};

} //end namespace inet


#endif /* INET_APPLICATIONS_BASICFLOODPROT_BANDWIDTHTWOPOINTS_H_ */
