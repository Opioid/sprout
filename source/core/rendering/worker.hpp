#pragma once

#include "rectangle.hpp"
#include "scene/shape/node_stack.hpp"
#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/random/generator.hpp"

namespace image { namespace texture { namespace sampler {

class Sampler_2D;

}}}

namespace sampler {

class Sampler;

}

namespace scene {

class Prop;
class Scene;
struct Intersection;

namespace camera {

class Camera;

}}

namespace rendering {

class Surface_integrator;
class Surface_integrator_factory;

class Worker {
public:

	Worker();
	~Worker();

	void init(uint32_t id, const math::random::Generator& rng, Surface_integrator_factory& surface_integrator_factory,
			  sampler::Sampler& sampler, const scene::Scene& scene);

	uint32_t id() const;

	void render(const scene::camera::Camera& camera, const Rectui& tile, uint32_t sample_begin, uint32_t sample_end,
				float normalized_tick_offset, float normalized_tick_slice);

	math::float4 li(math::Oray& ray);

	bool intersect(math::Oray& ray, scene::Intersection& intersection);
	bool intersect(const scene::Prop* prop, math::Oray& ray, scene::Intersection& intersection);

	bool visibility(const math::Oray& ray);

	bool intersect_m(math::Oray& ray, scene::Intersection& intersection);

	float masked_visibility(const math::Oray& ray, const image::texture::sampler::Sampler_2D& sampler);

	const scene::Scene& scene() const;

private:

	uint32_t id_;
	math::random::Generator rng_;
	Surface_integrator* surface_integrator_;
	sampler::Sampler* sampler_;
	const scene::Scene* scene_;
	scene::shape::Node_stack node_stack_;
};

}
