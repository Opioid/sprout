#ifndef SU_CORE_SCENE_WORKER_HPP
#define SU_CORE_SCENE_WORKER_HPP

#include "material/material_sample_cache.hpp"
#include "material/sampler_cache.hpp"
#include "shape/node_stack.hpp"
#include "take/take_settings.hpp"
#include "base/math/vector.hpp"
#include "base/random/generator.hpp"

namespace scene {

class Scene;
struct Ray;

namespace material { class Sample; }

namespace prop {
	class Prop;
	struct Intersection;
}

class Worker {

public:

	using Sampler_filter = material::Sampler_settings::Filter;
	using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
	using Texture_sampler_3D = image::texture::sampler::Sampler_3D;
	using Intersection = prop::Intersection;

	Worker();
	~Worker();

	void init(uint32_t id, take::Settings const& settings,
			  const Scene& scene, uint32_t max_sample_size);

	uint32_t id() const;

	bool intersect(Ray& ray, Intersection& intersection) const;
	bool intersect(Ray& ray, float& epsilon) const;
	bool intersect(const prop::Prop* prop, Ray& ray, Intersection& intersection) const;

	bool resolve_mask(Ray& ray, Intersection& intersection, Sampler_filter filter);

	bool intersect_and_resolve_mask(Ray& ray, Intersection& intersection,
									Sampler_filter filter);

	bool visibility(Ray const& ray) const;

	float masked_visibility(Ray const& ray, Sampler_filter filter) const;

	const Scene& scene() const;

	shape::Node_stack& node_stack() const;

	material::Sample_cache& sample_cache() const;

	template<typename T> T& sample() const;

	const Texture_sampler_2D& sampler_2D(uint32_t key, Sampler_filter filter) const;

	const Texture_sampler_3D& sampler_3D(uint32_t key, Sampler_filter filter) const;

private:

	uint32_t id_;

protected:

	rnd::Generator rng_;

	take::Settings settings_;

	const Scene* scene_;

	mutable shape::Node_stack node_stack_;

	mutable material::Sample_cache  sample_cache_;
	const   material::Sampler_cache sampler_cache_;
};

}

#endif
