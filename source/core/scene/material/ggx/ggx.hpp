#pragma once

#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace ggx {

template<typename Sample>
class GGX : public BxDF<Sample> {
public:

	GGX(const Sample& sample);

	math::float3 evaluate(const math::float3& wi, float n_dot_wi, float n_dot_wo) const;

	float pdf(const math::float3& wi, float n_dot_wi) const;

	float importance_sample(sampler::Sampler& sampler, BxDF_result& result) const;
};

math::float3 f(float wo_dot_h, const math::float3& f0);
float f(float wo_dot_h, float f0);

float d(float n_dot_h, float a2);

float g(float n_dot_wi, float n_dot_wo, float a2);

}}}
