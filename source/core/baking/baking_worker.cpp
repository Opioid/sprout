#include "baking_worker.hpp"
#include "baking_item.hpp"
#include "base/math/sampling.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.inl"

namespace baking {

void Baking_worker::baking_init(Item* items, int32_t slice_width) noexcept {
    items_       = items;
    slice_width_ = slice_width;
}

void Baking_worker::bake(int32_t begin, int32_t end) noexcept {
    for (int32_t i = begin; i < end;) {
        float2 const uv(rng_.random_float(), rng_.random_float());

        float3 const wi = sample_sphere_uniform(uv);

#ifdef BAKE_IMAGE
        int2 const c = coordinates(i);

        float const z = 0.f;

        float3 const p((float(c[0]) + 0.5f) / float(slice_width_) * 2.f - 1.f,
                       (float(c[1]) + 0.5f) / float(slice_width_) * -2.f + 1.f, z);
#else
        float3 const p(rng_.random_float() * 2.f - 1.f, rng_.random_float() * 2.f - 1.f,
                       rng_.random_float() * 2.f - 1.f);
#endif

        Intersection intersection;

        intersection.prop       = scene_->volume_entity(0);
        intersection.geo.part   = 0;
        intersection.geo.p      = p;
        intersection.geo.geo_n  = normalize(p);
        intersection.geo.uv     = float2(0.f);
        intersection.subsurface = true;

        interface_stack_.clear();
        interface_stack_.push(intersection);

        Interface_stack const temp_stack = interface_stack_;

        Ray ray(p, wi, 0.f, scene::Ray_max_t);

        float4 const li = surface_integrator_->li(ray, intersection, *this, temp_stack);

        items_[i].pos      = packed_float3(p);
        items_[i].wi       = packed_float3(wi);
        items_[i].radiance = li[0];

        ++i;
    }
}

int2 Baking_worker::coordinates(int32_t index) const noexcept {
    int2 c;
    c[1] = index / slice_width_;
    c[0] = index - c[1] * slice_width_;
    return c;
}

}  // namespace baking
