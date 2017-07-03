#pragma once

#include "density.hpp"

namespace scene { namespace volume {

class Height : public Density {

public:

	Height();

	virtual float3 optical_depth(const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Worker& worker, Sampler_filter filter) const override final;

private:

	virtual float density(const float3& p, Worker& worker,
						  Sampler_filter filter) const override final;

	virtual void set_parameter(const std::string& name, const json::Value& value) override final;

	float a_;
	float b_;
};

}}
