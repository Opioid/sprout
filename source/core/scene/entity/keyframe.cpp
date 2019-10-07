#include "keyframe.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/transformation.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_constants.hpp"

namespace scene::entity {

void Morphing::interpolate(Morphing& __restrict result, Morphing const& __restrict other,
                           float t) const noexcept {
    if (targets[0] == targets[1] && other.targets[0] == other.targets[1]) {
        result.weight = t;

        result.targets[0] = targets[0];
        result.targets[1] = other.targets[0];
    } else {
        result.weight = lerp(weight, other.weight, t);

        result.targets[0] = other.targets[0];
        result.targets[1] = other.targets[1];
    }
}

void Keyframe::interpolate(Keyframe& __restrict result, Keyframe const& __restrict other,
                           float t) const noexcept {
    result.transformation = lerp(transformation, other.transformation, t);
}

void Keyframe::transform(Keyframe& result, Transformation const& from) const noexcept {
    result.transformation.position = transform_point(from.object_to_world, transformation.position);

    result.transformation.scale = transformation.scale;

    result.transformation.rotation = quaternion::mul(quaternion::create(from.rotation),
                                                     transformation.rotation);

    result.time = time;
}

void Keyframe::transform(Keyframe& __restrict result, Keyframe const& __restrict from) const
    noexcept {
    result.transformation.position = transform_point(float4x4(from.transformation),
                                                     transformation.position);

    result.transformation.scale = transformation.scale;

    result.transformation.rotation = quaternion::mul(from.transformation.rotation,
                                                     transformation.rotation);

    result.time = scene::Static_time == from.time ? time : from.time;
}

}  // namespace scene::entity
