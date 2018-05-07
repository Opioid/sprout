#include "volumetric_octree.hpp"
#include "scene/scene_ray.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"

namespace scene::material::volumetric {

Box::Box(int3 const& min, int3 const& max) : bounds{min, max} {}

Octree::Octree() : root_{{nullptr, nullptr, nullptr, nullptr,
						  nullptr, nullptr, nullptr, nullptr},
						 0.f}
{}

bool Octree::intersect(Ray& ray, float& majorant_mu_t) const {
	Box box(int3(0), (dimensions_));

	return intersect(ray, &root_, box, majorant_mu_t);
}

bool Octree::intersect(Ray& ray, Build_node const* node, Box const& box,
					   float& majorant_mu_t) const {
//	int3 hounds0 = box.bounds[0];
//	hounds0[1] = dimensions_[1] - hounds0[1];



	float3 min = float3(2.f) * (float3(box.bounds[0]) / float3(dimensions_)) - float3(1.f);
	float3 max = float3(2.f) * (float3(box.bounds[1]) / float3(dimensions_)) - float3(1.f);


	math::AABB aabb(min, max);

	float hit_t;
	if (aabb.intersect_inside(ray, hit_t)) {
		ray.max_t = hit_t;

		if (!node->children[0]) {
			majorant_mu_t = node->majorant_mu_t;
			return true;
		}

		int3 const half = (box.bounds[1] - box.bounds[0]) / 2;

		Box const sub0(box.bounds[0], box.bounds[0] + half);
		intersect(ray, node->children[0], sub0, majorant_mu_t);

		Box const sub1(int3(box.bounds[0][0] + half[0], box.bounds[0][1], box.bounds[0][2]),
					   int3(box.bounds[1][0], box.bounds[0][1] + half[1], box.bounds[0][2] + half[2]));
		intersect(ray, node->children[1], sub1, majorant_mu_t);

		Box const sub2(int3(box.bounds[0][0], box.bounds[0][1] + half[1], box.bounds[0][2]),
					   int3(box.bounds[0][0] + half[0], box.bounds[1][1], box.bounds[0][2] + half[2]));
		intersect(ray, node->children[2], sub2, majorant_mu_t);

		Box const sub3(int3(box.bounds[0][0] + half[0], box.bounds[0][1] + half[1], box.bounds[0][2]),
					   int3(box.bounds[1][0], box.bounds[1][1], box.bounds[0][2] + half[2]));
		intersect(ray, node->children[3], sub3, majorant_mu_t);

		Box const sub4(int3(box.bounds[0][0], box.bounds[0][1], box.bounds[0][2] + half[2]),
					   int3(box.bounds[0][0] + half[0], box.bounds[0][1] + half[1], box.bounds[1][2]));
		intersect(ray, node->children[4], sub4, majorant_mu_t);

		Box const sub5(int3(box.bounds[0][0] + half[0], box.bounds[0][1], box.bounds[0][2] + half[2]),
					   int3(box.bounds[1][0], box.bounds[0][1] + half[1], box.bounds[1][2]));
		intersect(ray, node->children[5], sub5, majorant_mu_t);

		Box const sub6(int3(box.bounds[0][0], box.bounds[0][1] + half[1], box.bounds[0][2] + half[2]),
					   int3(box.bounds[0][0] + half[0], box.bounds[1][1], box.bounds[1][2]));
		intersect(ray, node->children[6], sub6, majorant_mu_t);

		Box const sub7(int3(box.bounds[0] + half), box.bounds[1]);
		intersect(ray, node->children[7], sub7, majorant_mu_t);

		return true;
	}

	return false;
}

}
