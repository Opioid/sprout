#pragma once

#include "Material_sample_cache.hpp"
#include "base/math/vector.hpp"

namespace scene {

namespace shape {

struct Differential;

}

namespace material {

class Sample;

class IMaterial {
public:

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo, uint32_t worker_id) = 0;
};

template<typename T>
class Material : public IMaterial {
public:

	Material(Sample_cache<T>& cache) : cache_(cache) {}

protected:

	Sample_cache<T>& cache_;
};

}}
