#ifndef SU_CORE_BAKING_ITEM_HPP
#define SU_CORE_BAKING_ITEM_HPP

#include "base/math/vector3.hpp"

namespace baking {

struct Item {
    float3 pos;
    float3 wi;
    float  radiance;
};

}  // namespace baking

#endif
