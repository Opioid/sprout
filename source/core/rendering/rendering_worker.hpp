#pragma once

#include "scene/shape/node_stack.hpp"
#include "base/math/vector.hpp"
#include "base/math/rectangle.hpp"
#include "base/math/random/generator.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace sampler { class Sampler; }

namespace scene {

class Prop;
class Scene;
struct Intersection;
struct Ray;

namespace camera { class Camera; }

}

namespace rendering {

namespace integrator {

namespace surface {

class Integrator;
class Integrator_factory;

}

namespace volume {

class Integrator;
class Integrator_factory;

}}

class Worker {
public:

	Worker();
	~Worker();

	void init(uint32_t id, const math::random::Generator& rng,
			  integrator::surface::Integrator_factory& surface_integrator_factory,
			  integrator::volume::Integrator_factory& volume_integrator_factory,
			  sampler::Sampler& sampler, const scene::Scene& scene);

	uint32_t id() const;

	math::float4 li(scene::Ray& ray);
	math::vec3 surface_li(scene::Ray& ray);
	math::float4 volume_li(const scene::Ray& ray, math::vec3& transmittance);

	math::vec3 transmittance(const scene::Ray& ray);

	bool intersect(scene::Ray& ray, scene::Intersection& intersection);
	bool intersect(const scene::Prop* prop, scene::Ray& ray, scene::Intersection& intersection);

	bool visibility(const scene::Ray& ray);

	float masked_visibility(const scene::Ray& ray, const image::texture::sampler::Sampler_2D& sampler);

	const scene::Scene& scene() const;

	scene::shape::Node_stack& node_stack();

protected:

	integrator::surface::Integrator* surface_integrator_;
	integrator::volume::Integrator*  volume_integrator_;
	sampler::Sampler* sampler_;

private:

	uint32_t id_;
	math::random::Generator rng_;

	const scene::Scene* scene_;
	scene::shape::Node_stack node_stack_;
};

}
