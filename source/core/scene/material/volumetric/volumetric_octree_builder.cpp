#include "volumetric_octree_builder.hpp"
#include "image/texture/texture.hpp"
#include "base/math/vector3.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::volumetric {

void Octree_builder::build(image::texture::Texture const& texture) {
	const int3 d = texture.dimensions_3();

	Box box(int3(0), d);

	split(texture, box, 0);
}

Octree_builder::Box::Box(int3 const& min, int3 const& max) : bounds{min, max} {}

void Octree_builder::split(image::texture::Texture const& texture, Box const& box, uint32_t depth) {
	static uint32_t constexpr max_depth = 2;

	if (max_depth == depth) {
		float max_density = 0.f;
		for (int32_t z = box.bounds[0][2], mz = box.bounds[1][2]; z < mz; ++z) {
			for (int32_t y = box.bounds[0][1], my = box.bounds[1][1]; y < my; ++y) {
				for (int32_t x = box.bounds[0][0], mx = box.bounds[1][0]; x < mx; ++x) {
					max_density = std::max(texture.at_1(x, y, z), max_density);
				}
			}
		}

		std::cout << "md " << box.bounds[0] << ", " << box.bounds[1] << ": "
				  << max_density << std::endl;

		return;
	}

	++depth;

	int3 half = (box.bounds[1] - box.bounds[0]) / 2;

	{
		Box sub(box.bounds[0], box.bounds[0] + half);
		split(texture, sub, depth);
	}

	{
		Box sub(int3(box.bounds[0][0] + half[0], box.bounds[0][1], box.bounds[0][2]),
				int3(box.bounds[1][0], box.bounds[0][1] + half[1], box.bounds[0][2] + half[2]));

		split(texture, sub, depth);
	}

	{
		Box sub(int3(box.bounds[0][0], box.bounds[0][1] + half[1], box.bounds[0][2]),
				int3(box.bounds[0][0] + half[0], box.bounds[1][1], box.bounds[0][2] + half[2]));

		split(texture, sub, depth);
	}

	{
		Box sub(int3(box.bounds[0][0] + half[0], box.bounds[0][1] + half[1], box.bounds[0][2]),
				int3(box.bounds[1][0], box.bounds[1][1], box.bounds[0][2] + half[2]));

		split(texture, sub, depth);
	}

	{
		Box sub(int3(box.bounds[0][0], box.bounds[0][1], box.bounds[0][2] + half[2]),
				int3(box.bounds[0][0] + half[0], box.bounds[0][1] + half[1], box.bounds[1][2]));

		split(texture, sub, depth);
	}

	{
		Box sub(int3(box.bounds[0][0] + half[0], box.bounds[0][1], box.bounds[0][2] + half[2]),
				int3(box.bounds[1][0], box.bounds[0][1] + half[1], box.bounds[1][2]));

		split(texture, sub, depth);
	}

	{
		Box sub(int3(box.bounds[0][0], box.bounds[0][1] + half[1], box.bounds[0][2] + half[2]),
				int3(box.bounds[0][0] + half[0], box.bounds[1][1], box.bounds[1][2]));

		split(texture, sub, depth);
	}

	{
		Box sub(int3(box.bounds[0] + half),
				box.bounds[1]);

		split(texture, sub, depth);
	}
}

}
