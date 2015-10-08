#pragma once

#include "composed_transformation.hpp"
#include "keyframe.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace thread { class Pool; }

namespace scene { namespace entity {

class Entity {
public:

	bool transformation_at(float tick_delta, Composed_transformation& transformation) const;

	void set_transformation(const math::transformation& t, thread::Pool& pool);

	void set_beginning(const Keyframe& frame);

	void tick(const Keyframe& frame, thread::Pool& pool);

protected:

	virtual void on_set_transformation(thread::Pool& pool) = 0;

	Keyframe frame_b_;
	Keyframe frame_a_;

	Composed_transformation world_transformation_;

	bool animated_;
};

}}
