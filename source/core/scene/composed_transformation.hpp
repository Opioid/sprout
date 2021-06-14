#ifndef SU_CORE_SCENE_COMPOSED_TRANSFORMATION_HPP
#define SU_CORE_SCENE_COMPOSED_TRANSFORMATION_HPP

#include "base/math/matrix3x3.hpp"
#include "base/math/matrix4x4.hpp"
#include "base/math/vector3.hpp"

namespace math {
struct Transformation;
}

namespace scene {

struct alignas(64) Composed_transformation {
    void prepare(math::Transformation const& t);

    void set(math::Transformation const& t);

    void set_position(float3_p p);

    float4x4 object_to_world() const;

    float scale_x() const;
    float scale_y() const;
    float scale_z() const;

    float2 scale_xy() const;

    float3 scale() const;

    float3 world_to_object_point(float3_p p) const;
    float3 world_to_object_vector(float3_p v) const;
    float3 world_to_object_normal(float3_p v) const;

    float3 object_to_world_point(float3_p v) const;
    float3 object_to_world_vector(float3_p v) const;
    float3 object_to_world_normal(float3_p v) const;

    float4x4 world_to_object;
    float3x3 rotation;
    float3   position;
};

}  // namespace scene

#endif
