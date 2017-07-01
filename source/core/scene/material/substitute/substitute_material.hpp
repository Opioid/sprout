#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class Material : public Material_base {

public:

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;
};

}}}
