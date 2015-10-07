#include "animation_stage.hpp"
#include "animation.hpp"
#include "scene/entity/entity.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace animation {

Stage::Stage(entity::Entity* entity, Animation* animation) : entity_(entity), animation_(animation) {
	entity::Keyframe frame;
	animation->beginning(frame);
	entity_->set_beginning(frame);
}

void Stage::update() {
	entity::Keyframe frame;
	animation_->current_frame(frame);
	entity_->tick(frame);
}

}}
