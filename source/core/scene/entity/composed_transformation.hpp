#ifndef SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_HPP
#define SU_CORE_SCENE_ENTITY_COMPOSED_TRANSFORMATION_HPP

#include "base/math/matrix3x3.hpp"
#include "base/math/matrix4x4.hpp"
#include "base/math/vector3.hpp"

namespace math {
struct Transformation;
}

namespace scene::entity {

struct alignas(64) Composed_transformation {
    void set(math::Transformation const& t) noexcept;

    float4x4 object_to_world() const noexcept;

    float scale_x() const noexcept;
    float scale_y() const noexcept;
    float scale_z() const noexcept;

    float2 scale_xy() const noexcept;

    float3 scale() const noexcept;

    float3 world_to_object_point(float3 const& p) const noexcept;
    float3 world_to_object_vector(float3 const& v) const noexcept;

    float3 object_to_world_point(float3 const& v) const noexcept;
    float3 object_to_world_vector(float3 const& v) const noexcept;

    float4x4 world_to_object;
    float3x3 rotation;
    float3   position;
};

}  // namespace scene::entity

#endif
