#pragma once

#include "material_sample.hpp"

namespace scene::material::testing {

bool check(float3 const& result, float3 const& h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
           float pdf, const Sample::Layer& layer);

bool check(float3 const& result, float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
           float pdf, const Sample::Layer& layer);

bool check(const bxdf::Sample& result, f_float3 wo, const Sample::Layer& layer);

bool check_normal_map(float3 const& n, float3 const& tangent_space_n, float2 uv);

void print_vector(float3 const& v);

}  // namespace scene::material::testing
