#pragma once

#include "material_sample.hpp"

namespace scene { namespace material { namespace testing {

bool check(float3_p result, float3_p wi, float3_p wo, float pdf, const Sample::Layer& layer);

bool check(const bxdf::Result& result, float3_p wo, const Sample::Layer& layer);

bool check_normal_map(float3_p n, float3_p tangent_space_n, float2 uv);

}}}
