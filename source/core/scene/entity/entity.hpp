#pragma once

#include "keyframe.hpp"
#include "composed_transformation.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace scene {

class Entity {
public:

	bool transformation_at(float time, Composed_transformation& transformation) const;

	void set_transformation(const math::transformation& t);

protected:

	virtual void on_set_transformation();

	Composed_transformation transformation_;

	Keyframe keyframe_a_;
	Keyframe keyframe_b_;
};

}
