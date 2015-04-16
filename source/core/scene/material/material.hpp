#pragma once

#include "material_sample_cache.hpp"
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

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id) = 0;
};

template<typename T>
class Material : public IMaterial {
public:

	Material(Sample_cache<T>& cache) : cache_(cache) {}

protected:

	Sample_cache<T>& cache_;
};

}}
