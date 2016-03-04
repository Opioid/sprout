#pragma once

#include "scene/shape/geometry/shape_intersection.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene {

class Worker;

namespace material {

class Sample;
class Material;

}

class Prop;

struct Intersection {
	material::Material* material() const;

	float opacity(Worker& worker, float time, material::Texture_filter override_filter) const;

	const material::Sample& sample(Worker& worker, math::pfloat3 wo, float time,
								   material::Texture_filter override_filter) const;

	const Prop* prop;
	shape::Intersection geo;
};

}
