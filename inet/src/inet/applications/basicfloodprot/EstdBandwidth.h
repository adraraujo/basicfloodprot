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
public:
    L3Address ipA;
    L3Address ipB;
    double bandwidth;

};


}
#endif /* INET_ESTDBANDWIDTH_H_ */
