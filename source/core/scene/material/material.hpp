#pragma once

#include "material_sample_cache.hpp"
#include "image/texture/texture_2d.hpp"
#include "base/math/vector.hpp"

namespace image { namespace sampler {

class Sampler_2D;

}}

namespace scene {

namespace shape {

struct Differential;

}

namespace material {

class Sample;

class IMaterial {
public:

	IMaterial(std::shared_ptr<image::Image> mask);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id) = 0;

	virtual math::float3 sample_emission() const = 0;

	float opacity(math::float2 uv, const image::sampler::Sampler_2D& sampler) const;

private:

	image::Texture_2D mask_;
};

template<typename T>
class Material : public IMaterial {
public:

	Material(Sample_cache<T>& cache, std::shared_ptr<image::Image> mask) : IMaterial(mask), cache_(cache) {}

protected:

	Sample_cache<T>& cache_;
};

}}
