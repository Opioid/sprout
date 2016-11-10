#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Density : public Volume {

public:

	virtual float3 optical_depth(const math::Oray& ray, float step_size,
								 random::Generator& rng, Worker& worker,
								 Sampler_filter filter) const final override;

	virtual float3 scattering(float3_p p, Worker& worker,
							  Sampler_filter filter) const final override;

protected:

	// expects p in object space!
	virtual float density(float3_p p, Worker& worker,
						  Sampler_filter filter) const = 0;

private:


};

}}
