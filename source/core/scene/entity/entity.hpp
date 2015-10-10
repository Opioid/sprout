#pragma once

#include "composed_transformation.hpp"
#include "keyframe.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace thread { class Pool; }

namespace scene { namespace entity {

class Entity {
public:

	Entity();
	virtual ~Entity();

	bool transformation_at(float tick_delta, Composed_transformation& transformation) const;

	void set_transformation(const math::transformation& t, thread::Pool& pool);

	void set_beginning(const Keyframe& frame);

	void tick(const Keyframe& frame, thread::Pool& pool);

	void attach(Entity* node);
	void detach();

	const Entity* parent() const;

protected:

	void add_sibling(Entity* node);
	void detach(Entity* node);
	void remove_sibling(Entity* node);

	virtual void on_set_transformation(thread::Pool& pool) = 0;

	Keyframe frame_b_;
	Keyframe frame_a_;

	Composed_transformation world_transformation_;

	bool animated_;

	Entity* parent_;
	Entity* next_;
	Entity* child_;
};

}}
