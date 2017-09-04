#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Homogeneous : public Volume {

public:

	virtual float3 optical_depth(const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, Worker& worker) const override final;

	virtual float3 scattering(const float3& p, Sampler_filter filter,
							  Worker& worker) const override final;

private:

	virtual void set_parameter(const std::string& name,
							   const json::Value& value) override final;

};

}}
