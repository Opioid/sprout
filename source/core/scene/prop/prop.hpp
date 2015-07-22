#pragma once

#include "scene/entity/entity.hpp"
#include "scene/shape/node_stack.hpp"
#include "scene/material/material.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"
#include <memory>
#include <vector>

namespace scene {

namespace shape {

struct Intersection;
class Shape;

}

class Prop : public entity::Entity {
public:

	virtual ~Prop();

	void init(std::shared_ptr<shape::Shape> shape, const material::Materials& materials);

	bool intersect(math::Oray& ray, shape::Node_stack& node_stack, shape::Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray, shape::Node_stack& node_stack) const;

	float opacity(const math::Oray& ray, shape::Node_stack& node_stack, const image::texture::sampler::Sampler_2D& sampler) const;

	const shape::Shape* shape() const;
	shape::Shape* shape();

	const math::aabb& aabb() const;

	material::IMaterial* material(uint32_t index) const;

	bool has_masked_material() const;
	bool has_emissive_material() const;
	bool has_emission_mapped_material() const;

private:

	virtual void on_set_transformation() final override;

	std::shared_ptr<shape::Shape> shape_;

	// Pre-transformed AABB in world space.
	// For moving objects it must cover the entire area occupied by the object during the tick.
	math::aabb aabb_;

	material::Materials materials_;

	bool has_masked_material_;
};

}
