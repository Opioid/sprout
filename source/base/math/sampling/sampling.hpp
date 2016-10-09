#pragma once

#include "math/vector.hpp"
#include "math/matrix.hpp"

namespace math {

float2 sample_disk_concentric(float2 uv);

float2 sample_triangle_uniform(float2 uv);

float3 sample_hemisphere_uniform(float2 uv);

float3 sample_oriented_hemisphere_uniform(float2 uv, float3_p x, float3_p y, float3_p z);
float3 sample_oriented_hemisphere_uniform(float2 uv, const float3x3& m);
float3 sample_oriented_hemisphere_uniform(float2 uv, const float4x4& m);

float3 sample_hemisphere_cosine(float2 uv);

float3 sample_oriented_hemisphere_cosine(float2 uv, float3_p x, float3_p y, float3_p z);

float3 sample_sphere_uniform(float2 uv);

float3 sample_oriented_cone_uniform(float2 uv, float cos_theta_max,
									float3_p x, float3_p y, float3_p z);

float cone_pdf_uniform(float cos_theta_max);

}
