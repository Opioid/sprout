#pragma once

#include "shape/node_stack.hpp"
#include "material/sampler_cache.hpp"
#include "base/math/vector3.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene {

class Prop;
class Scene;
struct Intersection;
struct Ray;

class Worker {

public:

	using Sampler_filter = material::Sampler_settings::Filter;
	using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
	using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

	Worker();
	~Worker();

	void init(uint32_t id, const Scene& scene);

	uint32_t id() const;

	bool intersect(Ray& ray, Intersection& intersection);
	bool intersect(const Prop* prop, Ray& ray, Intersection& intersection);

	bool visibility(const Ray& ray);

	float masked_visibility(const Ray& ray, Sampler_filter filter);

	float3 tinted_visibility(const Ray& ray, Sampler_filter filter);

	const Scene& scene() const;

	shape::Node_stack& node_stack();

	const Texture_sampler_2D& sampler_2D(uint32_t key, Sampler_filter filter) const;

	const Texture_sampler_3D& sampler_3D(uint32_t key, Sampler_filter filter) const;

private:

	uint32_t id_;

protected:

	const Scene* scene_;

	shape::Node_stack node_stack_;

	const material::Sampler_cache sampler_cache_;
};

}
