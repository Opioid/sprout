#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Density : public Volume {

public:

	virtual float3 optical_depth(const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, Worker& worker) const override;

	virtual float3 scattering(const float3& p, Sampler_filter filter,
							  Worker& worker) const override final;

protected:

	// expects p in object space!
	virtual float density(const float3& p, Sampler_filter filter, Worker& worker) const = 0;

private:

};

}}
