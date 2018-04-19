#include "flow_vis.hpp"
#include "tracking.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/heatmap.hpp"
#include "base/spectrum/rgb.hpp"

#include "base/debug/assert.hpp"

#include <iostream>

namespace rendering::integrator::volume {

Flow_vis::Flow_vis(rnd::Generator& rng, const take::Settings& take_settings,
				   const Settings& settings) :
	Integrator(rng, take_settings),
	settings_(settings) {}

void Flow_vis::prepare(const scene::Scene& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Flow_vis::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

float3 Flow_vis::transmittance(const Ray& /*ray*/, Worker& /*worker*/) {
	return float3(1.f);
}

bool Flow_vis::integrate(Ray& ray, Intersection& intersection,
						 Sampler_filter filter, Worker& worker,
						 float3& li, float3& transmittance, float3& weight) {
	if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
		li = float3(0.f);
		transmittance = float3(1.f);
		weight = float3(1.f);
		return false;
	}

	SOFT_ASSERT(!worker.interface_stack().empty());

	const auto interface = worker.interface_stack().top();

	const auto& material = *interface->material();

	Transformation temp;
	const auto& transformation = interface->prop->transformation_at(ray.time, temp);

	const float3 fv = material.emission(transformation, ray, 0.01f, rng_, filter, worker);
	const float a = 8.f * spectrum::luminance(fv);
	const float opacity = std::min(a, 1.f);
	transmittance = float3(1.f  - opacity);
	li = fv;

//	const float3 fv = material.emission(transformation, ray, 0.01f, rng_, filter, worker);
//	const float luminance = std::min(16.f * math::average(li), 1.f);
//	transmittance = math::saturate(float3(1.f - 4.f * fv));
//	li = float3(0.);

	weight = float3(1.);

	return true;
}

size_t Flow_vis::num_bytes() const {
	return sizeof(*this);
}

Flow_vis_factory::Flow_vis_factory(const take::Settings& settings, uint32_t num_integrators,
								   float step_size) :
	Factory(settings, num_integrators),
	integrators_(memory::allocate_aligned<Flow_vis>(num_integrators)),
	settings_{step_size} {}

Flow_vis_factory::~Flow_vis_factory() {
	memory::free_aligned(integrators_);
}

Integrator* Flow_vis_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&integrators_[id]) Flow_vis(rng, take_settings_, settings_);
}

}
