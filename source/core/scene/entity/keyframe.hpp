#ifndef SU_CORE_SCENE_ENTITY_KEYFRAME_HPP
#define SU_CORE_SCENE_ENTITY_KEYFRAME_HPP

#include "base/math/transformation.hpp"

namespace scene::entity {

struct alignas(16) Keyframe {
    void interpolate(Keyframe& result, Keyframe const& other, float t) const noexcept;

    void transform(math::Transformation& result, math::Transformation const& from) const noexcept;

    void transform(Keyframe& result, Keyframe const& from) const noexcept;

    math::Transformation transformation;

    struct Morphing {
        uint32_t targets[2];
        float    weight;
    };

    Morphing morphing;

    uint64_t time;
};

}  // namespace scene::entity

#endif
