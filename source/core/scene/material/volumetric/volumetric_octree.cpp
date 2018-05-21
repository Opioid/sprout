#include "volumetric_octree.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::volumetric {

Octree::Octree() : num_nodes_(0), nodes_(nullptr) {}

Octree::~Octree() {
	memory::free_aligned(nodes_);
}

Octree::Node* Octree::allocate_nodes(uint32_t num_nodes) {
	if (num_nodes != num_nodes_) {
		num_nodes_ = num_nodes;

		memory::free_aligned(nodes_);
		nodes_ = memory::allocate_aligned<Node>(num_nodes);
	}

	return nodes_;
}

void Octree::set_dimensions(int3 const& dimensions) {
	dimensions_ = dimensions;
	inv_2_dimensions_ = 2.f / float3(dimensions);
}

bool Octree::is_valid() const {
	return 0 != num_nodes_;
}

bool Octree::intersect(math::Ray& ray, float& majorant_mu_t) const {
//	{
//		Box const box{{int3(0), dimensions_}};

//		return intersect(ray, 0, box, majorant_mu_t);
//	}

	float3 const p = ray.point(ray.min_t) + float3(1.f);

	if (math::any_negative(p)) {
		return false;
	}

	int3 const v = int3((0.5f * float3(dimensions_)) * p);

	if (math::any_greater_equal(v, dimensions_)) {
		return false;
	}

	Box box{{int3(0), dimensions_}};

	uint32_t index = 0;

	for (uint32_t l = 0;; ++l) {		
		uint32_t children = nodes_[index].children;

		if (0 == children) {
			break;
		}

		index = children;

		int3 const half = (box.bounds[1] - box.bounds[0]) / 2;

		int3 const middle = box.bounds[0] + half;

		if (v[0] < middle[0]) {
			box.bounds[1][0] = middle[0];
		} else {
			box.bounds[0][0] = middle[0];
			index += 1;
		}

		if (v[1] < middle[1]) {
			box.bounds[1][1] = middle[1];
		} else {
			box.bounds[0][1] = middle[1];
			index += 2;
		}

		if (v[2] < middle[2]) {
			box.bounds[1][2] = middle[2];
		} else {
			box.bounds[0][2] = middle[2];
			index += 4;
		}
	}

	float3 const min = inv_2_dimensions_ * float3(box.bounds[0]) - float3(1.f);
	float3 const max = inv_2_dimensions_ * float3(box.bounds[1]) - float3(1.f);

	math::AABB aabb(min, max);

	float hit_t;
	if (aabb.intersect_inside(ray, hit_t)) {
		if (ray.max_t > hit_t) {
			ray.max_t = hit_t;
		}
	} else {
		ray.max_t = ray.min_t;
		return false;
	}

	majorant_mu_t = nodes_[index].majorant_mu_t;

	return true;
}

bool Octree::intersect_f(math::Ray& ray, float& majorant_mu_t) const {
//	{
//		Box const box{{int3(0), dimensions_}};

//		return intersect(ray, 0, box, majorant_mu_t);
//	}

	math::AABB box(float3(-1.f), float3(1.f));

	float3 p = ray.point(ray.min_t);

	if (math::any_lesser_equal(p, -1.f) || math::any_greater_equal(p, 1.f)) {
		float hit_t;
		if (!box.intersect_p(ray, hit_t)) {
			return false;
		}

		ray.min_t = hit_t;
		p = ray.point(ray.min_t);
	}

	uint32_t index = 0;

	for (uint32_t l = 0;; ++l) {
		uint32_t children = nodes_[index].children;

		if (0 == children) {
			break;
		}

		index = children;

		float3 const half = box.halfsize();

		float3 const middle = box.bounds[0] + half;

		if (p[0] < middle[0]) {
			box.bounds[1][0] = middle[0];
		}
		else {
			box.bounds[0][0] = middle[0];
			index += 1;
		}

		if (p[1] < middle[1]) {
			box.bounds[1][1] = middle[1];
		}
		else {
			box.bounds[0][1] = middle[1];
			index += 2;
		}

		if (p[2] < middle[2]) {
			box.bounds[1][2] = middle[2];
		}
		else {
			box.bounds[0][2] = middle[2];
			index += 4;
		}
	}

	float hit_t;
	if (box.intersect_inside(ray, hit_t)) {
		if (ray.max_t > hit_t) {
			ray.max_t = hit_t;
		}

	//	std::cout << box.bounds[0] << " ";
	//	std::cout << p << std::endl;
	}
	else {	
	//	std::cout << "min: " << box.bounds[0] << std::endl;
	//	std::cout << "max: " << box.bounds[1] << std::endl;
	//	std::cout << "p: " << p << std::endl;

		ray.max_t = ray.min_t;
		return false;
	}

	majorant_mu_t = nodes_[index].majorant_mu_t;

	return true;
}

bool Octree::intersect(math::Ray& ray, uint32_t node_id, Box const& box,
					   float& majorant_mu_t) const {
	float3 const min = inv_2_dimensions_ * float3(box.bounds[0]) - float3(1.f);
	float3 const max = inv_2_dimensions_ * float3(box.bounds[1]) - float3(1.f);

	math::AABB aabb(min, max);

	float hit_t;
	if (aabb.intersect_inside(ray, hit_t)) {
		if (ray.max_t > hit_t) {
			ray.max_t = hit_t;
		}

		auto const& node = nodes_[node_id];

		if (0 == node.children) {
			majorant_mu_t = node.majorant_mu_t;
			return true;
		}

		intersect_children(ray, node, box, majorant_mu_t);

		return true;
	}

	return false;
}

void Octree::intersect_children(math::Ray& ray, Node const& node, Box const& box,
								float& majorant_mu_t) const {
	int3 const half = (box.bounds[1] - box.bounds[0]) / 2;

	int3 const middle = box.bounds[0] + half;

	Box const sub0{{box.bounds[0], middle}};
	intersect(ray, node.children + 0, sub0, majorant_mu_t);

	Box const sub1{{int3(middle[0], box.bounds[0][1], box.bounds[0][2]),
					int3(box.bounds[1][0], middle[1], middle[2])}};
	intersect(ray, node.children + 1, sub1, majorant_mu_t);

	Box const sub2{{int3(box.bounds[0][0], middle[1], box.bounds[0][2]),
					int3(middle[0], box.bounds[1][1], middle[2])}};
	intersect(ray, node.children + 2, sub2, majorant_mu_t);

	Box const sub3{{int3(middle[0], middle[1], box.bounds[0][2]),
					int3(box.bounds[1][0], box.bounds[1][1], middle[2])}};
	intersect(ray, node.children + 3, sub3, majorant_mu_t);

	Box const sub4{{int3(box.bounds[0][0], box.bounds[0][1], middle[2]),
					int3(middle[0], middle[1], box.bounds[1][2])}};
	intersect(ray, node.children + 4, sub4, majorant_mu_t);

	Box const sub5{{int3(middle[0], box.bounds[0][1], middle[2]),
					int3(box.bounds[1][0], middle[1], box.bounds[1][2])}};
	intersect(ray, node.children + 5, sub5, majorant_mu_t);

	Box const sub6{{int3(box.bounds[0][0], middle[1], middle[2]),
					int3(middle[0], box.bounds[1][1], box.bounds[1][2])}};
	intersect(ray, node.children + 6, sub6, majorant_mu_t);

	Box const sub7{{middle, box.bounds[1]}};
	intersect(ray, node.children + 7, sub7, majorant_mu_t);
}

}
