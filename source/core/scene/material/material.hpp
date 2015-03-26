#pragma once

#include "Material_sample_cache.hpp"

namespace scene {

namespace shape {

struct Differential;

}

namespace material {

class Sample;

class IMaterial {
public:

	virtual const Sample& sample(const shape::Differential& dg, uint32_t worker_id) = 0;
};

template<typename T>
class Material : public IMaterial {
public:

	Material(Sample_cache<T>& cache) : cache_(cache) {}

protected:

	Sample_cache<T>& cache_;
};

}}
