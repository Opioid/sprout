#include "shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"

namespace scene::shape {

char const* Shape::identifier() {
    return "Shape";
}

Shape::Shape(Properties properties) : properties_(properties) {}

Shape::~Shape() = default;

AABB Shape::part_aabb(uint32_t /*part*/, uint32_t /*variant*/) const {
    return aabb();
}

uint32_t Shape::num_parts() const {
    return 1;
}

uint32_t Shape::num_materials() const {
    return 1;
}

uint32_t Shape::part_id_to_material_id(uint32_t part) const {
    return part;
}

bool Shape::sample_volume(uint32_t /*part*/, float3_p /*p*/, Transformation const& /*trafo*/,
                          float /*volume*/, Sampler& /*sampler*/, RNG& /*rng*/,
                          uint32_t /*sampler_d*/, Sample_to& /*sample*/) const {
    return false;
}

uint32_t Shape::prepare_sampling(uint32_t /*part*/, uint32_t /*material*/,
                                 light::Tree_builder& /*builder*/, Worker const& /*worker*/,
                                 Threads& /*threads*/) {
    return 0;
}

float4 Shape::cone(uint32_t /*part*/) const {
    return float4(0.f, 0.f, 1.f, -1.f);
}

Morphable* Shape::morphable_shape() {
    return nullptr;
}

}  // namespace scene::shape
