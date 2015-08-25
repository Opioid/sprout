#pragma once

#include "scene/material/material.hpp"
#include "substitute_sample_cache.hpp"

namespace scene { namespace material { namespace substitute {

class Substitute : public Material<Sample_cache> {
public:

	Substitute(Sample_cache& cache,
			   std::shared_ptr<image::texture::Texture_2D> mask);

	virtual math::float3 sample_emission(math::float2 uv,
										 const image::texture::sampler::Sampler_2D& sampler) const override;

	virtual math::float3 average_emission() const override;

	virtual const image::texture::Texture_2D* emission_map() const override;
};

}}}
