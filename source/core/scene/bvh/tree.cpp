#include "tree.hpp"
#include "scene/prop/prop.hpp"
#include <iostream>

namespace scene { namespace bvh {

Build_node::Build_node() {
	children[0] = nullptr;
	children[1] = nullptr;
}

Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}

bool Build_node::intersect(math::Oray& ray, const std::vector<Prop*>& props, Intersection& intersection) const {
	if (!aabb.intersect_p(ray)) {
		return false;
	}

	/*
hit := false

if n.children[0] != nil {
	c := ray.Sign[n.axis]

	if n.children[c].intersect(ray, visibility, props, scratch, intersection) {
		hit = true
	}

	if n.children[1 - c].intersect(ray, visibility, props, scratch, intersection) {
		hit = true
	}
} else {
	for i := n.offset; i < n.propsEnd; i++ {
		p := props[i]
		if p.IsVisible(visibility) && p.Intersect(ray, scratch, &intersection.Geo) {
			intersection.Prop = p
			hit = true
		}
	}
}

return hit*/

	bool hit = false;

	if (children[0]) {
		uint8_t c = ray.sign[axis];

		if (children[c]->intersect(ray, props, intersection)) {
			hit = true;
		}

		if (children[1 - c]->intersect(ray, props, intersection)) {
			hit = true;
		}
	} else {
		for (uint32_t i = offset; i < props_end; ++i) {
			auto p = props[i];
			if (p->intersect(ray, intersection.geo)) {
				intersection.prop = p;
				hit = true;
			}
		}
	}

	return hit;
}

bool Build_node::intersect_p(const math::Oray& ray, const std::vector<Prop*>& props) const {
	if (!aabb.intersect_p(ray)) {
		return false;
	}
/*
	if n.children[0] != nil {
		c := ray.Sign[n.axis]

		if n.children[c].intersectP(ray, props, scratch) {
			return true
		}

		return n.children[1 - c].intersectP(ray, props, scratch)
	}

	for i := n.offset; i < n.propsEnd; i++ {
		if props[i].CastsShadow && props[i].IntersectP(ray, scratch) {
			return true
		}
	}

	return false
	*/

	if (children[0]) {
		uint8_t c = ray.sign[axis];

		if (children[c]->intersect_p(ray, props)) {
			return true;
		}

		return children[1 - c]->intersect_p(ray, props);
	}

	for (uint32_t i = offset; i < props_end; ++i) {
		if (props[i]->intersect_p(ray)) {
			return true;
		}
	}

	return false;
}

bool Tree::intersect(math::Oray& ray, Intersection& intersection) const {
	bool hit = false;

	if (root_.intersect(ray, props_, intersection)) {
		hit = true;
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		auto p = props_[i];
		if (p->intersect(ray, intersection.geo)) {
			intersection.prop = p;
			hit = true;
		}
	}

	return hit;
}

bool Tree::intersect_p(const math::Oray& ray) const {
	if (root_.intersect_p(ray, props_)) {
		return true;
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		if (props_[i]->intersect_p(ray)) {
			return true;
		}
	}

	return false;
}

}}
