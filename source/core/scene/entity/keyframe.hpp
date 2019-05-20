#ifndef SU_CORE_SCENE_ENTITY_KEYFRAME_HPP
#define SU_CORE_SCENE_ENTITY_KEYFRAME_HPP

#include "base/math/transformation.hpp"

namespace scene::entity {

struct Morphing {
    void interpolate(Morphing& result, Morphing const& other, float t) const noexcept;

    uint32_t targets[2];
    float    weight;
};

struct alignas(16) Keyframe {
    void interpolate(Keyframe& result, Keyframe const& other, float t) const noexcept;

    void transform(math::Transformation& result, math::Transformation const& from) const noexcept;

    void transform(Keyframe& result, Keyframe const& from) const noexcept;

    math::Transformation transformation;

    uint64_t time;
};

}  // namespace scene::entity

#endif
