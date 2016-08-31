#pragma once

#include "scene/entity/entity.hpp"
#include "scene/material/material.hpp"
#include "base/flags/flags.hpp"
#include "base/math/bounding/aabb.hpp"
#include <memory>
#include <vector>

namespace thread { class Pool; }

namespace scene {

class Worker;
struct Ray;

namespace shape {

struct Intersection;
class Shape;
class Node_stack;

}

class Prop : public entity::Entity {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	virtual ~Prop();

	void set_shape_and_materials(std::shared_ptr<shape::Shape> shape,
								 const material::Materials& materials);

	virtual void set_parameters(const json::Value& parameters) final override;

	void set_visibility(bool in_camera, bool in_reflection, bool in_shadow);

	void prepare_sampling(uint32_t part, uint32_t light_id);

	void morph(thread::Pool& pool);

	bool intersect(Ray& ray, shape::Node_stack& node_stack,
				   shape::Intersection& intersection) const;

	bool intersect_p(const Ray& ray, shape::Node_stack& node_stack) const;

	float opacity(const Ray& ray, Worker& worker, Sampler_filter filter) const;

	const shape::Shape* shape() const;
	shape::Shape* shape();

	const math::aabb& aabb() const;

	float area(uint32_t part) const;

	const material::Materials& materials() const;
	material::Material* material(uint32_t part) const;

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

	struct Part {
		float    area;
		uint32_t light_id;
	};

	std::vector<Part> parts_;

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
