#include "null.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"

#include "base/debug/assert.hpp"
#include "shape_test.hpp"

namespace scene::shape {

Null::Null() : Shape(Properties(Property::Analytical)) {}

AABB Null::aabb() const {
    return Empty_AABB;
}

uint32_t Null::num_parts() const {
    return 0;
}

uint32_t Null::num_materials() const {
    return 0;
}

bool Null::intersect(Ray& /*ray*/, Transformation const& /*trafo*/, Node_stack& /*nodes*/,
                     Intersection& /*isec*/) const {
    return false;
}

bool Null::intersect_nsf(Ray& /*ray*/, Transformation const& /*trafo*/, Node_stack& /*nodes*/,
                         Intersection& /*isec*/) const {
    return false;
}

bool Null::intersect(Ray& /*ray*/, Transformation const& /*trafo*/, Node_stack& /*nodes*/,
                     Normals& /*normals*/) const {
    return false;
}

bool Null::intersect_p(Ray const& /*ray*/, Transformation const& /*trafo*/,
                       Node_stack& /*nodes*/) const {
    return false;
}

float Null::visibility(Ray const& /*ray*/, Transformation const& /*trafo*/, uint32_t /*entity*/,
                       Filter /*filter*/, Worker& /*worker*/) const {
    return 1.f;
}

bool Null::thin_absorption(Ray const& /*ray*/, Transformation const& /*trafo*/, uint32_t /*entity*/,
                           Filter /*filter*/, Worker& /*worker*/, float3& ta) const {
    ta = float3(0.f);
    return true;
}

bool Null::sample(uint32_t /*part*/, uint32_t /*variant*/, float3_p /*p*/, float3_p /*n*/,
                  Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                  bool /*total_sphere*/, Sampler& /*sampler*/, RNG& /*rng*/, uint32_t /*sampler_d*/,
                  Sample_to& /*sample*/) const {
    return false;
}

bool Null::sample(uint32_t /*part*/, uint32_t /*variant*/, Transformation const& /*trafo*/, float /*area*/,
                  bool /*two_sided*/, Sampler& /*sampler*/, RNG& /*rng*/, uint32_t /*sampler_d*/,
                  float2 /*importance_uv*/, AABB const& /*bounds*/, Sample_from& /*sample*/) const {
    return false;
}

float Null::pdf(uint32_t /*variant*/, Ray const& /*ray*/, float3_p /*n*/, Intersection const& /*isec*/,
                Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/,
                bool /*total_Null*/) const {
    return 0.f;
}

float Null::pdf_volume(Ray const& /*ray*/, Intersection const& /*isec*/,
                       Transformation const& /*trafo*/, float /*volume*/) const {
    return 0.f;
}

bool Null::sample(uint32_t /*part*/, float3_p /*p*/, float2 /*uv*/, Transformation const& /*trafo*/,
                  float /*area*/, bool /*two_sided*/, Sample_to& /*sample*/) const {
    return false;
}

bool Null::sample_volume(uint32_t /*part*/, float3_p /*p*/, float3_p /*uvw*/,
                         Transformation const& /*trafo*/, float /*volume*/,
                         Sample_to& /*sample*/) const {
    return false;
}

bool Null::sample(uint32_t /*part*/, float2 /*uv*/, Transformation const& /*trafo*/, float /*area*/,
                  bool /*two_sided*/, float2 /*importance_uv*/, AABB const& /*bounds*/,
                  Sample_from& /*sample*/) const {
    return false;
}

float Null::pdf_uv(Ray const& /*ray*/, Intersection const& /*isec*/,
                   Transformation const& /*trafo*/, float /*area*/, bool /*two_sided*/) const {
    return 0.f;
}

float Null::uv_weight(float2 /*uv*/) const {
    return 0.f;
}

float Null::area(uint32_t /*part*/, float3_p /*scale*/) const {
    return 0.f;
}

float Null::volume(uint32_t /*part*/, float3_p /*scale*/) const {
    return 0.f;
}

Shape::Differential_surface Null::differential_surface(uint32_t /*primitive*/) const {
    return {float3(1.f, 0.f, 0.f), float3(0.f, -1.f, 0.f)};
}

}  // namespace scene::shape
