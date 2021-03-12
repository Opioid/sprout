#ifndef SU_CORE_SCENE_ENTITY_KEYFRAME_HPP
#define SU_CORE_SCENE_ENTITY_KEYFRAME_HPP

#include "base/math/transformation.hpp"

namespace scene::entity {

struct Composed_transformation;

struct Morphing {
    void interpolate(Morphing& __restrict result, Morphing const& __restrict other, float t) const;

    uint32_t targets[2];
    float    weight;
};

struct alignas(16) Keyframe {
    void set(Keyframe const& other, float3_p camera_pos);

    void interpolate(Keyframe& __restrict result, Keyframe const& __restrict other, float t) const;

    using Transformation = Composed_transformation;

    void transform(Keyframe& result, Transformation const& from) const;

    void transform(Keyframe& __restrict result, Keyframe const& __restrict from) const;

    math::Transformation trafo;
};

}  // namespace scene::entity

#endif
