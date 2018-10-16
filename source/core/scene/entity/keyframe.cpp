#include "keyframe.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/transformation.inl"

namespace scene::entity {

void Keyframe::interpolate(Keyframe& result, Keyframe const& other, float t) const noexcept {
    result.transformation = math::lerp(transformation, other.transformation, t);

    if (morphing.targets[0] == morphing.targets[1] &&
        other.morphing.targets[0] == other.morphing.targets[1]) {
        result.morphing.weight = t;

        result.morphing.targets[0] = morphing.targets[0];
        result.morphing.targets[1] = other.morphing.targets[0];

    }
    /*	else if (morphing.targets[0] != other.morphing.targets[0]
                     ||  morphing.targets[1] != other.morphing.targets[1]) {
                    result.morphing.weight = other.morphing.weight;
            }*/
    else {
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

}  // namespace scene::entity
