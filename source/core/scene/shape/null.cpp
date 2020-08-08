#include "null.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Null::Null() : Shape(Properties(Property::Finite, Property::Analytical)) {}

float3 Null::object_to_texture_point(float3 const& p) const {
    return p;
}

float3 Null::object_to_texture_vector(float3 const& v) const {
    return v;
}

AABB Null::transformed_aabb(float4x4 const& /*m*/) const {
    return AABB::empty();
}

uint32_t Null::num_parts() const {
    return 0;
}

uint32_t Null::num_materials() const {
    return 0;
}

bool Null::intersect(Ray& /*ray*/, Transformation const& /*transformation*/,
                     Node_stack& /*node_stack*/, Intersection& /*intersection*/) const {
    return false;
}

bool Null::intersect_nsf(Ray& /*ray*/, Transformation const& /*transformation*/,
                         Node_stack& /*node_stack*/, Intersection& /*intersection*/) const {
    return false;
}

bool Null::intersect(Ray& /*ray*/, Transformation const& /*transformation*/,
                     Node_stack& /*node_stack*/, Normals& /*normals*/) const {
    return false;
}

bool Null::intersect_p(Ray const& /*ray*/, Transformation const& /*transformation*/,
                       Node_stack& /*node_stack*/) const {
    return false;
}

float Null::visibility(Ray const& /*ray*/, Transformation const& /*transformation*/,
                       uint32_t /*entity*/, Filter /*filter*/, Worker& /*worker*/) const {
    return 1.f;
}

bool Null::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
                           uint32_t /*entity*/, Filter /*filter*/, Worker& /*worker*/,
                           float3& ta) const {
    ta = float3(0.f);
    return true;
}

bool Null::sample(uint32_t /*part*/, float3 const& /*p*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                  uint32_t /*sampler_dimension*/, Sample_to& /*sample*/) const {
    return false;
}

bool Null::sample(uint32_t /*part*/, Transformation const& /*transformation*/, float /*area*/,
                  bool /*two_sided*/, Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                  uint32_t /*sampler_dimension*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                  Sample_from& /*sample*/) const {
    return false;
}

float Null::pdf(Ray const& /*ray*/, Intersection const& /*intersection*/,
                Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                bool /*total_Null*/) const {
    return 0.f;
}

float Null::pdf_volume(Ray const& /*ray*/, Intersection const& /*intersection*/,
                       Transformation const& /*transformation*/, float /*volume*/) const {
    return 0.f;
}

bool Null::sample(uint32_t /*part*/, float3 const& /*p*/, float2 /*uv*/,
                  Transformation const& /*transformation*/, float /*area*/, bool /*two_sided*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Null::sample(uint32_t /*part*/, float3 const& /*p*/, float3 const& /*uvw*/,
                  Transformation const& /*transformation*/, float /*volume*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Null::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*transformation*/,
                  float /*area*/, bool /*two_sided*/, float2 /*importance_uv*/,
                  AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Null::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
                   Transformation const& /*transformation*/, float /*area*/,
                   bool /*two_sided*/) const {
    return 0.f;
}

float Null::uv_weight(float2 /*uv*/) const {
    return 0.f;
}

float Null::area(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 0.f;
}

float Null::volume(uint32_t /*part*/, float3 const& /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Null::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

}  // namespace scene::shape
