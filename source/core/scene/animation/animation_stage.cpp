#include "animation_stage.hpp"
#include "animation.hpp"
#include "scene/entity/entity.hpp"

namespace scene { namespace animation {

Stage::Stage(entity::Entity* entity, Animation* animation) : entity_(entity), animation_(animation) {
	math::transformation t;
	animation->beginning(t);
	entity_->set_beginning(t);
}

void Stage::update() {
	math::transformation t;
	animation_->current_frame(t);
	entity_->tick(t);
}

}}
