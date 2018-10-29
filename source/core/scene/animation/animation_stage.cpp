#include "animation_stage.hpp"
#include "animation.hpp"
#include "base/math/vector4.inl"
#include "scene/entity/entity.hpp"

namespace scene::animation {

Stage::Stage(entity::Entity* entity, Animation* animation) noexcept
    : entity_(entity), animation_(animation) {}

void Stage::allocate_enitity_frames() const noexcept {
    entity_->allocate_frames(animation_->num_interpolated_frames());
}

void Stage::update() const noexcept {
    entity_->set_frames(animation_->interpolated_frames(), animation_->num_interpolated_frames());
}

}  // namespace scene::animation
