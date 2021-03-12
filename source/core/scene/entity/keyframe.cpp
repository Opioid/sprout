#include "keyframe.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/transformation.inl"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene_constants.hpp"

namespace scene::entity {

void Morphing::interpolate(Morphing& __restrict result, Morphing const& __restrict other,
                           float t) const {
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

void Keyframe::set(Keyframe const& other, float3_p camera_pos) {
    trafo.position = other.trafo.position - camera_pos;
    trafo.scale    = other.trafo.scale;
    trafo.rotation = other.trafo.rotation;
}

void Keyframe::interpolate(Keyframe& __restrict result, Keyframe const& __restrict other,
                           float t) const {
    result.trafo = lerp(trafo, other.trafo, t);
}

void Keyframe::transform(Keyframe& result, Transformation const& from) const {
    result.trafo.position = from.object_to_world_point(trafo.position);

    result.trafo.scale = trafo.scale;

    result.trafo.rotation = quaternion::mul(quaternion::create(from.rotation), trafo.rotation);
}

void Keyframe::transform(Keyframe& __restrict result, Keyframe const& __restrict from) const {
    result.trafo.position = transform_point(float4x4(from.trafo), trafo.position);

    result.trafo.scale = trafo.scale;

    result.trafo.rotation = quaternion::mul(from.trafo.rotation, trafo.rotation);
}

}  // namespace scene::entity
