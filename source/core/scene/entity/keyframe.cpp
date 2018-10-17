#include "keyframe.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/transformation.inl"
#include "scene/scene_constants.hpp"

namespace scene::entity {

void Keyframe::interpolate(Keyframe& result, Keyframe const& other, float t) const noexcept {
    result.transformation = math::lerp(transformation, other.transformation, t);

    if (morphing.targets[0] == morphing.targets[1] &&
        other.morphing.targets[0] == other.morphing.targets[1]) {
        result.morphing.weight = t;

        result.morphing.targets[0] = morphing.targets[0];
        result.morphing.targets[1] = other.morphing.targets[0];

    } else {
        result.morphing.weight = math::lerp(morphing.weight, other.morphing.weight, t);

        result.morphing.targets[0] = other.morphing.targets[0];
        result.morphing.targets[1] = other.morphing.targets[1];
    }
}

void Keyframe::transform(math::Transformation& result, math::Transformation const& from) const
    noexcept {
    result.position = math::transform_point(float4x4(from), transformation.position);
    result.rotation = math::quaternion::mul(transformation.rotation, from.rotation);
    result.scale    = transformation.scale;
}

void Keyframe::transform(Keyframe& result, Keyframe const& from) const noexcept {
    result.transformation.position = math::transform_point(float4x4(from.transformation),
                                                           transformation.position);
    result.transformation.rotation = math::quaternion::mul(from.transformation.rotation,
                                                           transformation.rotation);
    result.transformation.scale    = transformation.scale;
    result.time                    = scene::Static_time == from.time ? time : from.time;
}

}  // namespace scene::entity
