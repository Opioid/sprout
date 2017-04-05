#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Density : public Volume {

public:

	virtual float3 optical_depth(const math::Ray& ray, float step_size,
								 rnd::Generator& rng, Worker& worker,
								 Sampler_filter filter) const override;

	virtual float3 scattering(const float3& p, Worker& worker,
							  Sampler_filter filter) const final override;

protected:

	// expects p in object space!
	virtual float density(const float3& p, Worker& worker, Sampler_filter filter) const = 0;

private:

};

}}
