#ifndef SU_CORE_SCENE_ENTITY_KEYFRAME_HPP
#define SU_CORE_SCENE_ENTITY_KEYFRAME_HPP

#include "base/math/transformation.hpp"

namespace scene::entity {

struct Composed_transformation;

struct Morphing {
    void interpolate(Morphing& __restrict result, Morphing const& __restrict other, float t) const
        noexcept;

    uint32_t targets[2];
    float    weight;
};

struct alignas(16) Keyframe {
    void interpolate(Keyframe& __restrict result, Keyframe const& __restrict other, float t) const
        noexcept;

    using Transformation = Composed_transformation;

    void transform(Keyframe& result, Transformation const& from) const noexcept;

    void transform(Keyframe& __restrict result, Keyframe const& __restrict from) const noexcept;

    math::Transformation transformation;

    uint64_t time;
};

}  // namespace scene::entity

#endif
