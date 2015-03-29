#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material { namespace ggx {

math::float3 f(float wo_dot_h, const math::float3& f0);

float d(float n_dot_h, float a2);

float g(float n_dot_wi, float n_dot_wo, float a2);

}}}
