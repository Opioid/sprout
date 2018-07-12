#include "flow_vis.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/heatmap.hpp"
#include "base/spectrum/rgb.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"
#include "tracking.hpp"

#include "base/debug/assert.hpp"

#include <iostream>

namespace rendering::integrator::volume {

Flow_vis::Flow_vis(rnd::Generator& rng, take::Settings const& take_settings,
                   Settings const& settings)
    : Integrator(rng, take_settings), settings_(settings) {}

void Flow_vis::prepare(scene::Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) {}

void Flow_vis::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

bool Flow_vis::transmittance(Ray const& /*ray*/, Worker& /*worker*/, float3& transmittance) {
    transmittance = float3(1.f);
    return true;
}

bool Flow_vis::integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                         Worker& worker, float3& li, float3& transmittance) {
    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        li            = float3(0.f);
        transmittance = float3(1.f);
        //	weight = float3(1.f);
        return false;
    }

    SOFT_ASSERT(!worker.interface_stack().empty());

    auto const interface = worker.interface_stack().top();

    auto const& material = *interface->material();

    Transformation temp;
    auto const&    transformation = interface->prop->transformation_at(ray.time, temp);

    float3 const fv      = material.emission(ray, transformation, 0.01f, rng_, filter, worker);
    float const  a       = 8.f * spectrum::luminance(fv);
    float const  opacity = std::min(a, 1.f);
    transmittance        = float3(1.f - opacity);
    li                   = fv;

    //	float3 const fv = material.emission(transformation, ray, 0.01f, rng_, filter, worker);
    //	float const luminance = std::min(16.f * math::average(li), 1.f);
    //	transmittance = math::saturate(float3(1.f - 4.f * fv));
    //	li = float3(0.);

    //	weight = float3(1.);

    return true;
}

size_t Flow_vis::num_bytes() const {
    return sizeof(*this);
}

Flow_vis_factory::Flow_vis_factory(take::Settings const& settings, uint32_t num_integrators,
                                   float step_size)
    : Factory(settings, num_integrators),
      integrators_(memory::allocate_aligned<Flow_vis>(num_integrators)),
      settings_{step_size} {}

Flow_vis_factory::~Flow_vis_factory() {
    memory::free_aligned(integrators_);
}

Integrator* Flow_vis_factory::create(uint32_t id, rnd::Generator& rng) const {
    return new (&integrators_[id]) Flow_vis(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::volume
