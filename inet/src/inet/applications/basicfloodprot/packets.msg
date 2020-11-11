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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;


cplusplus {{
#include <vector>
#include "inet/networklayer/common/L3Address.h"
using namespace inet;
typedef std::vector<inet::L3Address> Path;
}}

class Path {
    @existingClass;
}

class L3Address {
    @existingClass;
}
//
// TODO generated message class
//
class PathPayload extends inet::FieldsChunk
{
    Path path;
    L3Address target;    
    int flowId;
}