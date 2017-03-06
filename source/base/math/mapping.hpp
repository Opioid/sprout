#pragma once

#include "vector3.hpp"

namespace math {

float3 disk_to_hemisphere_equidistant(float2 uv);

float2 hemisphere_to_disk_equidistant(float3_p dir);

float2 hemisphere_to_disk_paraboloid(float3_p dir);

}
