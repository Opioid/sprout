#include "animation_stage.hpp"
#include "animation.hpp"
#include "scene/entity/entity.hpp"
#include "base/math/vector3.inl"

namespace scene { namespace animation {

Stage::Stage(entity::Entity* entity, Animation* animation) :
	entity_(entity), animation_(animation) {}

void Stage::update() {
	entity_->tick(animation_->interpolated_frame());
}

}}
