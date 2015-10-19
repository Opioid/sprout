#include "animation_stage.hpp"
#include "animation.hpp"
#include "scene/entity/entity.hpp"

namespace scene { namespace animation {

Stage::Stage(entity::Entity* entity, Animation* animation) : entity_(entity), animation_(animation) {
	entity_->set_beginning(animation->beginning());
}

void Stage::update() {
	entity_->tick(animation_->interpolated_frame());
}

}}
