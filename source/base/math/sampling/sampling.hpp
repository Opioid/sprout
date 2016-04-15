#pragma once

#include "math/vector.hpp"

namespace math {

float2 sample_disk_concentric(float2 uv);

float2 sample_triangle_uniform(float2 uv);

float3 sample_hemisphere_uniform(float2 uv);

float3 sample_oriented_hemisphere_uniform(float2 uv, pfloat3 x, pfloat3 y, pfloat3 z);

float3 sample_hemisphere_cosine(float2 uv);

float3 sample_oriented_hemisphere_cosine(float2 uv, pfloat3 x, pfloat3 y, pfloat3 z);

float3 sample_sphere_uniform(float2 uv);

float3 sample_oriented_cone_uniform(float2 uv, float cos_theta_max,
									const float3& x, const float3& y, const float3& z);

float cone_pdf_uniform(float cos_theta_max);

}
