#include "triangle_bvh_tree.hpp"
#include "scene/shape/triangle/triangle_primitive.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

const math::AABB& Tree::aabb() const {
	return aabb_;
}

bool Tree::intersect(math::Oray& ray, const math::float2& bounds, Intersection& intersection) const {
	bool hit = false;

	Coordinates c;

	Intersection ti;
	ti.c.t = ray.max_t;

	for (size_t i = 0; i < /*triangles_.size()*/8000; ++i) {
		if (triangles_[i].intersect(ray, c)) {
			if (c.t < ti.c.t) {
				ti.c = c;
				ti.index = i;
				hit = true;
			}
		}

	}

	if (hit) {
		intersection = ti;
		ray.max_t = ti.c.t;
	}

/*
ti := primitive.Intersection{}
ti.T = ray.MaxT

for i := node.startIndex; i < node.endIndex; i++ {
	if h, c := t.Triangles[i].Intersect(ray); h && c.T < ti.T {
		ti.Coordinates = c
		ti.Index = i
		hit = true
	}
}

if hit {
	// the idea was not to write to these pointers in the loop... Don't know whether it makes a difference
	*intersection = ti
	ray.MaxT = ti.T
}
	*/


	return hit;
}

bool Tree::intersect_p(const math::Oray& ray, const math::float2& bounds) const {
	for (size_t i = 0; i < /*triangles_.size()*/8000; ++i) {
		if (triangles_[i].intersect_p(ray)) {
			return true;
		}
	}

	return false;
}

}}}}




