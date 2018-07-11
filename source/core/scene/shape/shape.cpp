#include "shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"

namespace scene::shape {

Shape::~Shape() {}

math::AABB const& Shape::aabb() const {
    return aabb_;
}

float3 Shape::object_to_texture_point(f_float3 p) const {
    return (p - aabb_.bounds[0]) * inv_extent_;
}

float3 Shape::object_to_texture_vector(f_float3 v) const {
    return v * inv_extent_;
}

math::AABB Shape::transformed_aabb(float4x4 const& m, math::Transformation const& /*t*/) const {
    return aabb_.transform(m);
}

math::AABB Shape::transformed_aabb(math::Transformation const& t) const {
    return transformed_aabb(float4x4(t), t);
}

uint32_t Shape::num_parts() const {
    return 1;
}

// bool Shape::intersect_p(FVector ray_origin, FVector ray_direction,
//						FVector ray_min_t, FVector ray_max_t,
//						Transformation const& transformation,
//						Node_stack& node_stack) const {
//	return false;
//}

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

Morphable_shape* Shape::morphable_shape() {
    return nullptr;
}

}  // namespace scene::shape
