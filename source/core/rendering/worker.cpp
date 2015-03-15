#include "worker.hpp"
#include "rendering/sampler/camera_sample.hpp"
#include "rendering/film/film.hpp"
#include "rendering/integrator/integrator.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "scene/surrounding/surrounding.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "base/math/vector.inl"

namespace rendering {

Worker::Worker(Surface_integrator* surface_integrator) : surface_integrator_(surface_integrator) {}

void Worker::render(const scene::Scene& scene, const camera::Camera& camera, const Rectui& tile) {
	scene_ = &scene;

	auto& film = camera.film();

	auto& dimensions = film.dimensions();

	sampler::Camera_sample sample;
	math::Oray ray;

	for (uint32_t y = tile.start.y; y < tile.end.y; ++y) {
		for (uint32_t x = tile.start.x; x < tile.end.x; ++x) {

			uint32_t sample_id = 0;

			sample.coordinates = math::float2(static_cast<float>(x), static_cast<float>(y));

			camera.generate_ray(sample, ray);

			math::float3 color = li(sample_id, ray);

			film.add_sample(sample, color);
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

}
