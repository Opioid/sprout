#include "shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"

namespace scene::shape {

std::string Shape::identifier() noexcept {
    return "Shape";
}

Shape::~Shape() noexcept {}

/*
float3 Shape::object_to_texture_point(float3 const& p) const noexcept {
    return (p - aabb_.bounds[0]) * inv_extent_;
}

float3 Shape::object_to_texture_vector(float3 const& v) const noexcept {
    return v * inv_extent_;
}

AABB Shape::transformed_aabb(float4x4 const& m, math::Transformation const& t) const noexcept {
    return aabb_.transform(m);
}


AABB Shape::transformed_aabb(math::Transformation const& t) const noexcept {
    return transformed_aabb(float4x4(t), t);
}
*/
uint32_t Shape::num_parts() const noexcept {
    return 1;
}

bool Shape::sample(uint32_t part, float3 const& p, float3 const& /*n*/,
                   Transformation const& transformation, float area, bool two_sided,
                   Sampler& sampler, uint32_t sampler_dimension, Node_stack& node_stack,
                   Sample_to& sample) const noexcept {
    return this->sample(part, p, transformation, area, two_sided, sampler, sampler_dimension,
                        node_stack, sample);
}

bool Shape::sample_volume(uint32_t /*part*/, float3 const& /*p*/,
                          Transformation const& /*transformation*/, float /*volume*/,
                          Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
                          Node_stack& /*node_stack*/, Sample_to& /*sample*/) const noexcept {
    return false;
}

bool Shape::is_complex() const noexcept {
    return false;
}

bool Shape::is_finite() const noexcept {
    return true;
}

bool Shape::is_analytical() const noexcept {
    return true;
}

void Shape::prepare_sampling(uint32_t /*part*/) noexcept {}

Morphable_shape* Shape::morphable_shape() noexcept {
    return nullptr;
}

}  // namespace scene::shape
