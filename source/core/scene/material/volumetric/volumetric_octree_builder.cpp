#include "volumetric_octree_builder.hpp"
#include "image/texture/texture.hpp"
#include "base/math/vector3.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::volumetric {

void Octree_builder::build(Octree& tree, image::texture::Texture const& texture,
						   float max_extinction) {
	const int3 d = texture.dimensions_3();

	Box box(int3(0), d);

	split(&tree.root_, box, texture, max_extinction, 0);

	tree.set_dimensions(d);
	tree.dimensions_ = d;
}

void Octree_builder::split(Build_node* node, Box const& box, image::texture::Texture const& texture,
						   float max_extinction, uint32_t depth) {
	static uint32_t constexpr max_depth = 3;

	float min_density = 1.f;
	float max_density = 0.f;
	for (int32_t z = box.bounds[0][2], mz = box.bounds[1][2]; z < mz; ++z) {
		for (int32_t y = box.bounds[0][1], my = box.bounds[1][1]; y < my; ++y) {
			for (int32_t x = box.bounds[0][0], mx = box.bounds[1][0]; x < mx; ++x) {
				float const density = texture.at_1(x, y, z);
				min_density = std::min(density, min_density);
				max_density = std::max(density, max_density);
			}
		}
	}

	if (max_depth == depth || 0.f == max_density) {
		node->majorant_mu_t = max_density * max_extinction;

		for (uint32_t i = 0; i < 8; ++i) {
			node->children[i] = nullptr;
		}

		std::cout << box.bounds[0] << ", " << box.bounds[1] << ": "
				  << "min = " << min_density * max_extinction << ", "
				  << "max = " << node->majorant_mu_t << std::endl;

		return;
	}

	++depth;

	int3 const half = (box.bounds[1] - box.bounds[0]) / 2;

	{
		Box const sub(box.bounds[0], box.bounds[0] + half);

		node->children[0] = new Build_node;
		split(node->children[0], sub, texture, max_extinction, depth);
	}

	{
		Box const sub(int3(box.bounds[0][0] + half[0], box.bounds[0][1], box.bounds[0][2]),
					  int3(box.bounds[1][0], box.bounds[0][1] + half[1], box.bounds[0][2] + half[2]));

		node->children[1] = new Build_node;
		split(node->children[1], sub, texture, max_extinction, depth);
	}

	{
		Box const sub(int3(box.bounds[0][0], box.bounds[0][1] + half[1], box.bounds[0][2]),
					  int3(box.bounds[0][0] + half[0], box.bounds[1][1], box.bounds[0][2] + half[2]));

		node->children[2] = new Build_node;
		split(node->children[2], sub, texture, max_extinction, depth);
	}

	{
		Box const sub(int3(box.bounds[0][0] + half[0], box.bounds[0][1] + half[1], box.bounds[0][2]),
					  int3(box.bounds[1][0], box.bounds[1][1], box.bounds[0][2] + half[2]));

		node->children[3] = new Build_node;
		split(node->children[3], sub, texture, max_extinction, depth);
	}

	{
		Box const sub(int3(box.bounds[0][0], box.bounds[0][1], box.bounds[0][2] + half[2]),
					  int3(box.bounds[0][0] + half[0], box.bounds[0][1] + half[1], box.bounds[1][2]));

		node->children[4] = new Build_node;
		split(node->children[4], sub, texture, max_extinction, depth);
	}

	{
		Box const sub(int3(box.bounds[0][0] + half[0], box.bounds[0][1], box.bounds[0][2] + half[2]),
					  int3(box.bounds[1][0], box.bounds[0][1] + half[1], box.bounds[1][2]));

		node->children[5] = new Build_node;
		split(node->children[5], sub, texture, max_extinction, depth);
	}

	{
		Box const sub(int3(box.bounds[0][0], box.bounds[0][1] + half[1], box.bounds[0][2] + half[2]),
					  int3(box.bounds[0][0] + half[0], box.bounds[1][1], box.bounds[1][2]));

		node->children[6] = new Build_node;
		split(node->children[6], sub, texture, max_extinction, depth);
	}

	{
		Box const sub(int3(box.bounds[0] + half), box.bounds[1]);

		node->children[7] = new Build_node;
		split(node->children[7], sub, texture, max_extinction, depth);
	}
}

}