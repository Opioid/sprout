#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_HPP

#include "base/math/plane.hpp"
#include "base/math/vector.hpp"

namespace scene::shape::triangle::bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c);

float3 triangle_max(float3_p a, float3_p b, float3_p c);

Simdf triangle_min(Simdf_p a, Simdf_p b, Simdf_p c);

Simdf triangle_max(Simdf_p a, Simdf_p b, Simdf_p c);

float3 triangle_min(float3_p a, float3_p b, float3_p c, float3_p x);

float3 triangle_max(float3_p a, float3_p b, float3_p c, float3_p x);

float triangle_area(float3_p a, float3_p b, float3_p c);

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, Plane_p p);

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, Plane_p p);

}  // namespace scene::shape::triangle::bvh

#endif
