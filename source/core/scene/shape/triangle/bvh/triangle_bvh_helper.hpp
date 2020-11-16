#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_HPP

#include "base/math/plane.hpp"
#include "base/math/vector3.hpp"
#include "base/simd/simd.hpp"

namespace scene::shape::triangle::bvh {

float3 triangle_min(float3_p a, float3_p b, float3_p c);

float3 triangle_max(float3_p a, float3_p b, float3_p c);

Simd3f triangle_min(Simd3f const& a, Simd3f const& b, Simd3f const& c);

Simd3f triangle_max(Simd3f const& a, Simd3f const& b, Simd3f const& c);

float3 triangle_min(float3_p a, float3_p b, float3_p c, float3_p x);

float3 triangle_max(float3_p a, float3_p b, float3_p c, float3_p x);

float triangle_area(float3_p a, float3_p b, float3_p c);

uint32_t triangle_side(float3_p a, float3_p b, float3_p c, Plane const& p);

bool triangle_completely_behind(float3_p a, float3_p b, float3_p c, Plane const& p);

}  // namespace scene::shape::triangle::bvh

#endif
