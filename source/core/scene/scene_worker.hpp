#pragma once

#include "shape/node_stack.hpp"
#include "material/sampler_cache.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene {

class Prop;
class Scene;
struct Intersection;
struct Ray;

class Worker {
public:

	Worker();

	void init(uint32_t id, const Scene& scene);

	uint32_t id() const;

	bool intersect(Ray& ray, Intersection& intersection);
	bool intersect(const Prop* prop, Ray& ray, Intersection& intersection);

	bool visibility(const Ray& ray);

	float masked_visibility(const Ray& ray, material::Sampler_settings::Filter filter);

	const Scene& scene() const;

	shape::Node_stack& node_stack();

	const image::texture::sampler::Sampler_2D&
	sampler(uint32_t key, material::Sampler_settings::Filter filter) const;

private:

	uint32_t id_;

protected:

	const Scene* scene_;

	shape::Node_stack node_stack_;

	material::Sampler_cache sampler_cache_;
};

}
