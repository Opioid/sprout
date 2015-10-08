#pragma once

#include "composed_transformation.hpp"
#include "keyframe.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace scene { namespace entity {

class Entity {
public:

	bool transformation_at(float tick_delta, Composed_transformation& transformation) const;

	void set_transformation(const math::transformation& t);

	void set_beginning(const Keyframe& frame);

	void tick(const Keyframe& frame);

protected:

	virtual void on_set_transformation() = 0;

	math::transformation local_transformation_;

	Keyframe frame_b_;
	Keyframe frame_a_;

	Composed_transformation world_transformation_;

	bool animated_;
};

}}
