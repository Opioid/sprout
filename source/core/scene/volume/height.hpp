#pragma once

#include "density.hpp"

namespace scene { namespace volume {

class Height : public Density {

public:

	Height();

private:

	virtual float density(float3_p p, Worker& worker,
						  Sampler_filter filter) const final override;

	virtual void set_parameter(const std::string& name,
							   const json::Value& value) final override;

	float a_;
	float b_;
};

}}
