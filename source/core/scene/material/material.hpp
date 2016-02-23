#pragma once

#include "image/texture/texture_2d.hpp"
#include "base/math/vector.hpp"
#include <vector>

namespace image { namespace texture { namespace sampler {

class Sampler_2D;

}}}

namespace scene {

namespace shape {

struct Differential;

}

namespace material {

class Sample;

class IMaterial {
public:

	IMaterial(std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided);

	virtual void tick(float absolute_time, float time_slice);

	virtual const Sample& sample(const shape::Differential& dg, const math::vec3& wo, float time, float ior_i,
								 const image::texture::sampler::Sampler_2D& sampler, uint32_t worker_id) = 0;

	virtual math::vec3 sample_emission(math::float2 uv, float time,
										 const image::texture::sampler::Sampler_2D& sampler) const = 0;

	virtual math::vec3 average_emission() const = 0;

	virtual bool has_emission_map() const = 0;

	virtual math::float2 emission_importance_sample(math::float2 r2, float& pdf) const;

	virtual float emission_pdf(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const;

	virtual float opacity(math::float2 uv, float time, const image::texture::sampler::Sampler_2D& sampler) const;

	virtual void prepare_sampling(bool spherical);

	virtual bool is_animated() const;

	bool is_masked() const;
	bool is_emissive() const;
	bool is_two_sided() const;

protected:

	std::shared_ptr<image::texture::Texture_2D> mask_;

	bool two_sided_;
};

template<typename Sample_cache>
class Material : public IMaterial {
public:

	Material(Sample_cache& cache, std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
		IMaterial(mask, two_sided), cache_(cache) {}

protected:

	Sample_cache& cache_;
};

typedef std::vector<std::shared_ptr<material::IMaterial>> Materials;

}}
