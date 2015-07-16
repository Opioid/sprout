#pragma once

#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace oren_nayar {

template<typename Sample>
class Oren_nayar : public BxDF<Sample> {
public:

	Oren_nayar(const Sample& sample);

	math::float3 evaluate(const math::float3& wi, float n_dot_wi, float n_dot_wo) const;

	float pdf(const math::float3& wi, float n_dot_wi) const;

	float importance_sample(sampler::Sampler& sampler, float n_dot_wo, BxDF_result& result) const;
};

}}}

