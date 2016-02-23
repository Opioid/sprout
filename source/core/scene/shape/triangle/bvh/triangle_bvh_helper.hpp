#pragma once

#include "base/math/vector.hpp"
#include "base/math/plane.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

math::vec3 triangle_min(const math::vec3& a, const math::vec3& b, const math::vec3& c);
math::vec3 triangle_max(const math::vec3& a, const math::vec3& b, const math::vec3& c);

math::vec3 triangle_min(const math::vec3& a, const math::vec3& b, const math::vec3& c, const math::vec3& x);
math::vec3 triangle_max(const math::vec3& a, const math::vec3& b, const math::vec3& c, const math::vec3& x);

float triangle_area(const math::vec3& a, const math::vec3& b, const math::vec3& c);

uint32_t triangle_side(const math::vec3& a, const math::vec3& b, const math::vec3& c, const math::plane& p);

bool triangle_completely_behind(const math::vec3& a, const math::vec3& b, const math::vec3& c,
								const math::plane& p);

}}}}
