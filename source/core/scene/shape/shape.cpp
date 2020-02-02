#include "shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"

namespace scene::shape {

char const* Shape::identifier() {
    return "Shape";
}

Shape::~Shape() = default;

uint32_t Shape::num_parts() const {
    return 1;
}

uint32_t Shape::num_materials() const {
    return 1;
}

uint32_t Shape::part_id_to_material_id(uint32_t part) const {
    return part;
}

bool Shape::sample(uint32_t part, float3 const& p, float3 const& /*n*/,
                   Transformation const& transformation, float area, bool two_sided,
                   Sampler& sampler, uint32_t sampler_dimension, Node_stack& node_stack,
                   Sample_to& sample) const {
    return this->sample(part, p, transformation, area, two_sided, sampler, sampler_dimension,
                        node_stack, sample);
}

bool Shape::sample_volume(uint32_t /*part*/, float3 const& /*p*/,
                          Transformation const& /*transformation*/, float /*volume*/,
                          Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                          Node_stack& /*node_stack*/, Sample_to& /*sample*/) const {
    return false;
}

bool Shape::is_complex() const {
    return false;
}

bool Shape::is_finite() const {
    return true;
}

bool Shape::is_analytical() const {
    return true;
}

void Shape::prepare_sampling(uint32_t /*part*/) {}

float3 Shape::center(uint32_t /*part*/) const {
    return float3(0.f);
}

Morphable* Shape::morphable_shape() {
    return nullptr;
}

}  // namespace scene::shape
