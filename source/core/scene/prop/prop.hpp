#pragma once

#include "scene/entity/entity.hpp"
#include "scene/material/material.hpp"
#include "base/flags/flags.hpp"
#include "base/math/bounding/aabb.hpp"
#include <memory>
#include <vector>

namespace thread { class Pool; }

namespace scene {

struct Ray;

namespace shape {

struct Intersection;
class Shape;
class Node_stack;

}

class Prop : public entity::Entity {
public:

	virtual ~Prop();

	void init(std::shared_ptr<shape::Shape> shape, const material::Materials& materials);

	void set_visibility(bool in_camera, bool in_reflection, bool in_shadow);

	bool intersect(scene::Ray& ray, shape::Node_stack& node_stack, shape::Intersection& intersection) const;

	bool intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const scene::Ray& ray, shape::Node_stack& node_stack,
				  const image::texture::sampler::Sampler_2D& sampler) const;

	const shape::Shape* shape() const;
	shape::Shape* shape();

	void morph(thread::Pool& pool);

	const math::aabb& aabb() const;

	const material::Materials& materials() const;
	material::IMaterial* material(uint32_t index) const;

	bool has_masked_material() const;

	bool is_open() const;
	void set_open(bool open);

	bool visible_in_camera() const;
	bool visible_in_reflection() const;
	bool visible_in_shadow() const;

private:

	bool visible(uint32_t ray_depth) const;

	virtual void on_set_transformation() final override;

	std::shared_ptr<shape::Shape> shape_;

	// Pre-transformed AABB in world space.
	// For moving objects it must cover the entire area occupied by the object during the tick.
	math::aabb aabb_;

	material::Materials materials_;

	enum class Properties {
		Visible_in_camera		= 1 << 0,
		Visible_in_reflection	= 1 << 1,
		Visible_in_shadow		= 1 << 2,
		Masked_material			= 1 << 3,
		Open					= 1 << 4
	};

	flags::Flags<Properties> properties_;
};

}
