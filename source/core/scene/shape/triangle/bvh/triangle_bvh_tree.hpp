#pragma once

#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace scene { namespace shape { namespace triangle {

struct Intersection;
struct Triangle;

namespace bvh {

class Tree  {
public:

	const math::AABB& aabb() const;

	bool intersect(math::Oray& ray, const math::float2& bounds, Intersection& intersection) const;
	bool intersect_p(const math::Oray& ray, const math::float2& bounds) const;

private:

	math::AABB aabb_;

	std::vector<Triangle> triangles_;

	friend class Builder;
};

}}}}
