#pragma once

#include "material_sample.hpp"

namespace scene::material::testing {

bool check(const float3& result, const float3& h,
		   float n_dot_wi, float n_dot_wo, float wo_dot_h, float pdf,
		   const Sample::Layer& layer);

bool check(const float3& result,
		   float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h, float pdf,
		   const Sample::Layer& layer);

bool check(const bxdf::Sample& result, const float3& wo, const Sample::Layer& layer);

bool check_normal_map(const float3& n, const float3& tangent_space_n, float2 uv);

void print_vector(const float3& v);

}
