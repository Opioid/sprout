#pragma once

#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace lambert {

template<typename Sample>
class Lambert : public BxDF<Sample> {
public:

	Lambert(const Sample& sample);

	virtual math::float3 evaluate(const math::float3& wi) const final override;
	virtual void importance_sample(sampler::Sampler& sampler, BxDF_result& result) const final override;
};


}}}
