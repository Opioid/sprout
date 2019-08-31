#include "null.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Null::Null() noexcept {}

float3 Null::object_to_texture_point(float3 const& p) const noexcept {
    return p;
}

float3 Null::object_to_texture_vector(float3 const& v) const noexcept {
    return v;
}

AABB Null::transformed_aabb(float4x4 const& /*m*/, math::Transformation const& t) const noexcept {
    return transformed_aabb(t);
}

AABB Null::transformed_aabb(math::Transformation const& /*t*/) const noexcept {
    return AABB::empty();
}

uint32_t Null::num_parts() const noexcept {
    return 0;
}

bool Null::intersect(Ray& /*ray*/, Transformation const& /*transformation*/,
                     Node_stack& /*node_stack*/, Intersection& /*intersection*/) const noexcept {
    return false;
}

bool Null::intersect_fast(Ray& /*ray*/, Transformation const& /*transformation*/,
                          Node_stack& /*node_stack*/, Intersection& /*intersection*/) const
    noexcept {
    return false;
}

bool Null::intersect(Ray& /*ray*/, Transformation const& /*transformation*/,
                     Node_stack& /*node_stack*/, Normals& /*normals*/) const noexcept {
    return false;
}

bool Null::intersect_p(Ray const& /*ray*/, Transformation const& /*transformation*/,
                       Node_stack& /*node_stack*/) const noexcept {
    return false;
}

float Null::opacity(Ray const& /*ray*/, Transformation const& /*transformation*/,
                    Materials /*materials*/, Filter /*filter*/, Worker const& /*worker*/) const
    noexcept {
    return 0.f;
}

bool Null::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
                           Materials /*materials*/, Filter /*filter*/, Worker const& /*worker*/,
                           float3& ta) const noexcept {
    ta = float3(0.f);
    return true;
}

bool Null::sample(uint32_t /*part*/, float3 const& /*p*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/,
                  uint32_t /*sampler_dimension*/, Node_stack& /*node_stack*/,
                  Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Null::sample(uint32_t /*part*/, Transformation const& /*transformation*/, float /*area*/,
                  bool /*two_sided*/, Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                  float2 /*importance_uv*/, AABB const& /*bounds*/, Node_stack& /*node_stack*/,
                  Sample_from& /*sample*/) const noexcept {
    return false;
}

float Null::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                bool /*total_Null*/) const noexcept {
    return 0.f;
}

float Null::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                       Transformation const& /*transformation*/, float /*volume*/) const noexcept {
    return 0.f;
}

bool Null::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Null::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                  Transformation const& /*transformation*/, float /*volume*/,
                  Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Null::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/,
                  uint32_t /*sampler_dimension*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                  Sample_from& /*sample*/) const noexcept {
    return false;
}

float Null::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                   Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/) const noexcept {
    return 0.f;
}

float Null::uv_weight(float2 /*uv*/) const noexcept {
    return 0.f;
}

float Null::area(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

float Null::volume(uint32_t /*part*/, float3 const& /*scale*/) const noexcept {
    return 0.f;
}

size_t Null::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::shape
