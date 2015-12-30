#pragma once

#include "composed_transformation.hpp"
#include "keyframe.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace scene { namespace entity {

class Entity {
public:

	Entity();
	virtual ~Entity();

	bool transformation_at(float tick_delta, Composed_transformation& transformation) const;

	void set_transformation(const math::transformation& t);

	void tick(const Keyframe& frame);

	void calculate_world_transformation();

	void attach(Entity* node);
	void detach();

	const Entity* parent() const;

protected:

	void propagate_transformation() const;
	void inherit_transformation(const math::transformation& a, const math::transformation& b);

	void add_sibling(Entity* node);
	void detach(Entity* node);
	void remove_sibling(Entity* node);

	virtual void on_set_transformation() = 0;

	Keyframe local_frame_a_;
	Keyframe local_frame_b_;

	math::transformation world_frame_a_;
	math::transformation world_frame_b_;

	Composed_transformation world_transformation_;

	bool animated_;

	Entity* parent_;
	Entity* next_;
	Entity* child_;
};

}}
