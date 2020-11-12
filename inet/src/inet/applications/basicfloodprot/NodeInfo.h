/*
 * NodeInfo.h
 *
 *  Created on: 11 de nov de 2020
 *      Author: Adriano Araújo
 */

#ifndef INET_NODEINFO_H_
#define INET_NODEINFO_H_

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/common/geometry/common/Coord.h"

#include "BasicFloodProt.h"


namespace inet {

class BasicFloodProt;

class INET_API NodeInfo {
public:
    L3Address ip;
    Coord position;
};

}
#endif /* INET_NODEINFO_H_ */
