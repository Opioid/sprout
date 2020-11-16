#pragma once

#include "material_sample.hpp"

namespace scene::material::testing {

bool check(float3_p result, float3_p h, float n_dot_wi, float n_dot_wo, float wo_dot_h, float pdf,
           const Layer& layer);

bool check(float3_p result, float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
           float pdf, const Layer& layer);

bool check(float3_p result, float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
           float pdf);

bool check(const bxdf::Sample& result, float3_p wo, const Layer& layer);

bool check(const bxdf::Sample& result, float3_p wo, float n_dot_wi, float n_dot_wo, float wo_dot_h,
           const Layer& layer);

bool check_normal_map(float3_p n, float3_p tangent_space_n, float2 uv);

void print_vector(float3_p v);

}  // namespace scene::material::testing
