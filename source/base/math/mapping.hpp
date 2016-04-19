#pragma once

#include "vector.hpp"

namespace math {

math::float3 disk_to_hemisphere_equidistant(math::float2 uv);

math::float2 hemisphere_to_disk_equidistant(math::pfloat3 dir);

math::float2 hemisphere_to_disk_paraboloid(math::pfloat3 dir);

}
