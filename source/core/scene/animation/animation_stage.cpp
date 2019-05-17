#include "animation_stage.hpp"
#include "animation.hpp"
#include "base/math/vector4.inl"
#include "scene/entity/entity.hpp"
#include "scene/scene.hpp"

namespace scene::animation {

Stage::Stage(uint32_t entity, Animation* animation) noexcept
    : entity_(entity), animation_(animation) {}

void Stage::allocate_enitity_frames(Scene& scene) const noexcept {
    uint32_t const num_frames = animation_->num_interpolated_frames();
    scene.entity(entity_)->allocate_frames(num_frames, num_frames);
}

void Stage::update(Scene& scene) const noexcept {
    scene.entity(entity_)->set_frames(animation_->interpolated_frames(),
                                      animation_->num_interpolated_frames());
}

}  // namespace scene::animation
