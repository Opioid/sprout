#pragma once

#include "sampler_settings.hpp"
#include "base/math/vector.hpp"
#include <memory>
#include <vector>

namespace image { namespace texture { class Texture_2D; }}

namespace scene {

class Worker;

namespace shape { struct Hitpoint; }

namespace material {

class Sample;

class Material {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Material(std::shared_ptr<image::texture::Texture_2D> mask,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual void tick(float absolute_time, float time_slice);

	virtual const Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
								 float area, float time, float ior_i,
								 const Worker& worker, Sampler_filter filter) = 0;

	virtual math::float3 sample_radiance(math::pfloat3 wi, math::float2 uv, float area, float time,
										 const Worker& worker, Sampler_filter filter) const;

	virtual math::float3 average_radiance() const;

	virtual bool has_emission_map() const;

	virtual math::float2 radiance_importance_sample(math::float2 r2, float& pdf) const;

	virtual float emission_pdf(math::float2 uv, const Worker& worker, Sampler_filter filter) const;

	virtual float opacity(math::float2 uv, float time, const Worker& worker,
						  Sampler_filter filter) const;

	virtual void prepare_sampling(bool spherical);

	virtual bool is_animated() const;

	uint32_t sampler_key() const;

	bool is_masked() const;
	bool is_emissive() const;
	bool is_two_sided() const;

protected:

	std::shared_ptr<image::texture::Texture_2D> mask_;

	uint32_t sampler_key_;

	bool two_sided_;
};

template<typename Sample_cache>
class Typed_material : public Material {

public:

	Typed_material(Sample_cache& cache, std::shared_ptr<image::texture::Texture_2D> mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
		Material(mask, sampler_settings, two_sided), cache_(cache) {}

protected:

	Sample_cache& cache_;
};

using Materials = std::vector<std::shared_ptr<material::Material>>;

}}
