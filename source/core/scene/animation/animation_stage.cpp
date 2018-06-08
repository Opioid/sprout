#include "animation_stage.hpp"
#include "animation.hpp"
#include "base/math/vector4.inl"
#include "scene/entity/entity.hpp"

namespace scene::animation {

Stage::Stage(entity::Entity* entity, Animation* animation)
    : entity_(entity), animation_(animation) {}

void Stage::update() {
    entity_->tick(animation_->interpolated_frame());
}

}  // namespace scene::animation
