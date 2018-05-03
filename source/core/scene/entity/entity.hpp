#ifndef SU_CORE_SCENE_ENTITY_ENTITY_HPP
#define SU_CORE_SCENE_ENTITY_ENTITY_HPP

#include "composed_transformation.hpp"
#include "keyframe.hpp"
#include "base/flags/flags.hpp"
#include "base/json/json_types.hpp"
#include "base/math/transformation.hpp"

namespace scene::entity {

class Entity {

public:

	using Transformation = Composed_transformation;

	Entity() = default;
	virtual ~Entity();

	virtual void set_parameters(json::Value const& parameters) = 0;

	const math::Transformation& local_frame_a() const;

	// Only the returned reference is guaranteed to contain the actual transformation data.
	// This might or might not be the same reference which is passed as a parameter,
	// depending on whether the entity is animated or not.
	// This can sometimes avoid a relatively costly copy,
	// while keeping the animated state out of the interface.
	const Transformation& transformation_at(float tick_delta, Transformation& transformation) const;

	void set_transformation(const math::Transformation& t);

	void tick(const Keyframe& frame);

	void calculate_world_transformation();

	bool visible_in_camera() const;
	bool visible_in_reflection() const;
	bool visible_in_shadow() const;

	void set_visible_in_shadow(bool value);
	void set_visibility(bool in_camera, bool in_reflection, bool in_shadow,
						bool propagate = false);
	void set_propagate_visibility(bool enable);

	void attach(Entity* node);
	void detach();

	const Entity* parent() const;

protected:

	void propagate_transformation() const;
	void inherit_transformation(const math::Transformation& a,
								const math::Transformation& b,
								bool animated);

	void add_sibling(Entity* node);
	void detach(Entity* node);
	void remove_sibling(Entity* node);

	virtual void on_set_transformation() = 0;

	enum class Property {
		Animated				= 1 << 0,
		Visible_in_camera		= 1 << 1,
		Visible_in_reflection	= 1 << 2,
		Visible_in_shadow		= 1 << 3,
		Propagate_visibility	= 1 << 4,
		Masked_material			= 1 << 5,
		Tinted_shadow			= 1 << 6
	};

	flags::Flags<Property> properties_;

	Composed_transformation world_transformation_;

	math::Transformation world_frame_a_;
	math::Transformation world_frame_b_;

	Keyframe local_frame_a_;
	Keyframe local_frame_b_;

	Entity* parent_ = nullptr;
	Entity* next_	= nullptr;
	Entity* child_	= nullptr;
};

}

#endif
