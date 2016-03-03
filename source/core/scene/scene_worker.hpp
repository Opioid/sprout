#pragma once

#include "scene/shape/node_stack.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene {

class Prop;
class Scene;
struct Intersection;
struct Ray;

class Worker {
public:

	Worker();

	void init(uint32_t id, const scene::Scene& scene);

	uint32_t id() const;

	bool intersect(scene::Ray& ray, scene::Intersection& intersection);
	bool intersect(const scene::Prop* prop, scene::Ray& ray, scene::Intersection& intersection);

	bool visibility(const scene::Ray& ray);

	float masked_visibility(const scene::Ray& ray, const image::texture::sampler::Sampler_2D& sampler);

	const scene::Scene& scene() const;

	scene::shape::Node_stack& node_stack();

private:

	uint32_t id_;

protected:

	const scene::Scene* scene_;

	scene::shape::Node_stack node_stack_;
};

}
