#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace substitute {

class Sample : public material::Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi) const;

	void set(const math::float3& color);

private:

	math::float3 color_;
};

class Substitute : public Material<Sample> {
public:

	Substitute(Sample_cache<Sample>& cache);

};

}}}
