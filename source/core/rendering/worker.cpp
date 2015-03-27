#include "worker.hpp"
#include "rendering/film/film.hpp"
#include "rendering/integrator/integrator.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "scene/surrounding/surrounding.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"

namespace rendering {

Worker::Worker(uint32_t id, const math::random::Generator& rng,
			   Surface_integrator_factory& surface_integrator_factory, sampler::Sampler& sampler) :
	rng_(rng), surface_integrator_(surface_integrator_factory.create(id, rng_)), sampler_(sampler.clone(rng_)) {}

Worker::~Worker() {
	delete sampler_;
}

void Worker::render(const scene::Scene& scene, const scene::camera::Camera& camera, const Rectui& tile) {
	scene_ = &scene;

	auto& film = camera.film();

	sampler::Camera_sample sample;
	math::Oray ray;

	for (uint32_t y = tile.start.y; y < tile.end.y; ++y) {
		for (uint32_t x = tile.start.x; x < tile.end.x; ++x) {
			sampler_->restart();

			uint32_t sample_id = 0;

			math::float2 offset(static_cast<float>(x), static_cast<float>(y));

			while (sampler_->generate_camera_sample(offset, sample)) {
				camera.generate_ray(sample, ray);

				math::float3 color = li(sample_id, ray);

				film.add_sample(sample, color, tile);

				++sample_id;
			}
		}
	}
}

math::float3 Worker::li(uint32_t subsample, math::Oray& ray) const {
	scene::Intersection intersection;
	bool hit = intersect(ray, intersection);
	if (hit) {
		return surface_integrator_->li(*this, subsample, ray, intersection);
	} else {
		return scene_->surrounding()->sample(ray);
	}
}

bool Worker::intersect(math::Oray& ray, scene::Intersection& intersection) const {
	return scene_->intersect(ray, intersection);
}

bool Worker::visibility(const math::Oray& ray) const {
	return !scene_->intersect_p(ray);
}

const scene::Scene& Worker::scene() const {
	return *scene_;
}

}
