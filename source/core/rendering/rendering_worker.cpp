#include "rendering_worker.hpp"
#include "rendering/film/film.hpp"
#include "rendering/integrator/integrator.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "scene/camera/camera.hpp"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/material/material.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/math/random/generator.inl"

namespace rendering {

Worker::Worker() : sampler_(nullptr), node_stack_(128) {}

Worker::~Worker() {
	delete sampler_;
}

void Worker::init(uint32_t id, const math::random::Generator& rng,
				  Surface_integrator_factory& surface_integrator_factory,
				  sampler::Sampler& sampler, const scene::Scene& scene) {
	id_ = id;
	rng_ = rng;
	surface_integrator_ = surface_integrator_factory.create(rng_);
	sampler_ = sampler.clone();
	scene_ = &scene;
}

uint32_t Worker::id() const {
	return id_;
}

math::float4 Worker::li(math::Oray& ray) {
	scene::Intersection intersection;
	bool hit = intersect(ray, intersection);
	if (hit) {
		return surface_integrator_->li(*this, ray, intersection);
	} else {
		return math::float4::identity;
	}
}

bool Worker::intersect(math::Oray& ray, scene::Intersection& intersection) {
	return scene_->intersect(ray, node_stack_, intersection);
}

bool Worker::intersect(const scene::Prop* prop, math::Oray& ray, scene::Intersection& intersection) {
	bool hit = prop->intersect(ray, node_stack_, intersection.geo);
	if (hit) {
		intersection.prop = prop;
	}

	return hit;
}

bool Worker::visibility(const math::Oray& ray) {
	return !scene_->intersect_p(ray, node_stack_);
}

float Worker::masked_visibility(const math::Oray& ray, const image::texture::sampler::Sampler_2D& sampler) {
	return 1.f - scene_->opacity(ray, node_stack_, sampler);
}

const scene::Scene& Worker::scene() const {
	return *scene_;
}

scene::shape::Node_stack& Worker::node_stack() {
	return node_stack_;
}

void Camera_worker::render(const scene::camera::Camera& camera, const Rectui& tile,
						   uint32_t sample_begin, uint32_t sample_end,
						   float normalized_tick_offset, float normalized_tick_slice) {
	auto& film = camera.film();

	uint32_t num_samples = sample_end - sample_begin;

	sampler::Camera_sample sample;
	math::Oray ray;

	for (uint32_t y = tile.start.y; y < tile.end.y; ++y) {
		for (uint32_t x = tile.start.x; x < tile.end.x; ++x) {
			if (0 == sample_begin) {
				film.set_seed(x, y, sampler_->restart(1));
			} else {
				sampler_->set_seed(film.seed(x, y));
			}

			surface_integrator_->start_new_pixel(num_samples);

			math::uint2 pixel(x, y);

			for (uint32_t i = sample_begin; i < sample_end; ++i) {
				sampler_->generate_camera_sample(pixel, i, sample);

				camera.generate_ray(sample, normalized_tick_offset, normalized_tick_slice, ray);

				math::float4 color = li(ray);

				film.add_sample(sample, color, tile);
			}
		}
	}
}

}
