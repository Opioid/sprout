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

	void set_visibility(bool primary, bool secondary);

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

	bool primary_visibility() const;
	bool secondary_visibility() const;

private:

	bool visible(uint32_t ray_depth) const;

	virtual void on_set_transformation() final override;

	std::shared_ptr<shape::Shape> shape_;

	// Pre-transformed AABB in world space.
	// For moving objects it must cover the entire area occupied by the object during the tick.
	math::aabb aabb_;

	material::Materials materials_;

	enum class Properties {
		Primary_visibility		= 1 << 0,
		Secondary_visibility	= 1 << 1,
		Has_masked_material	    = 1 << 2,
		Is_open					= 1 << 3
	};

	flags::Flags<Properties> properties_;
};

}
