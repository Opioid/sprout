#include "baking_worker.hpp"
#include "baking_item.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.inl"
//#include "rendering/integrator/volume/volume_integrator.hpp"
#include "base/math/sampling.inl"

namespace baking {

void Baking_worker::baking_init(Item* items) noexcept {
    items_ = items;
}

void Baking_worker::bake(int32_t begin, int32_t end) noexcept {
    for (int32_t i = begin; i < end;) {
        float2 const uv(rng_.random_float(), rng_.random_float());

        float3 const wi = sample_sphere_uniform(uv);

        float3 const p(rng_.random_float() * 2.f - 1.f, rng_.random_float() * 2.f - 1.f,
                       rng_.random_float() * 2.f - 1.f);

        Intersection intersection;

        intersection.prop       = scene_->volume_entity(0);
        intersection.geo.part   = 0;
        intersection.geo.p      = p;
        intersection.geo.geo_n  = normalize(p);
        intersection.geo.uv     = float2(0.f);
        intersection.subsurface = true;

        interface_stack_.clear();
        interface_stack_.push(intersection);

        Interface_stack temp_stack = interface_stack_;

        Ray ray(p, wi, 0.f, scene::Ray_max_t);

        float4 const li = surface_integrator_->li(ray, intersection, *this, temp_stack);

        items_[i].pos      = p;
        items_[i].wi       = wi;
        items_[i].radiance = li[0];

        ++i;
    }
}

}  // namespace baking
