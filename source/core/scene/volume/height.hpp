#pragma once

#include "density.hpp"

namespace scene { namespace volume {

class Height : public Density {

public:

	virtual float3 optical_depth(const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, Worker& worker) const override final;

private:

	virtual float density(const float3& p, Sampler_filter filter,
						  Worker& worker) const override final;

	virtual void set_parameter(const std::string& name, const json::Value& value) override final;

	float a_ = 1.f;
	float b_ = 1.f;
};

}}
