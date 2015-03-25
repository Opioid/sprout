#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"
#include <memory>
#include <vector>

namespace rendering { namespace material {

class Material;

}}

namespace scene {

namespace shape {

struct Intersection;
class Shape;

}

class Prop : public Entity {
public:

	typedef std::vector<std::shared_ptr<rendering::material::Material>> Materials;

	virtual ~Prop();

	void init(std::shared_ptr<shape::Shape> shape, const Materials& materials);

	bool intersect(math::Oray& ray, shape::Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray) const;

	const shape::Shape* shape() const;

	const math::AABB& aabb() const;

	rendering::material::Material* material(uint32_t index) const;

private:

	virtual void on_set_transformation();

	std::shared_ptr<shape::Shape> shape_;

	math::AABB aabb_;

	Materials materials_;
};

}
