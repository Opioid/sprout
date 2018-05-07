#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"

namespace image::texture { class Texture; }

namespace scene {

struct Ray;

namespace material::volumetric {

struct Build_node {
	Build_node* children[8];

	float majorant_mu_t;
};

struct Box {
	Box() = default;
	Box(int3 const& min, int3 const& max);

	int3 bounds[2];
};

class Octree {

public:

	Octree();

	bool intersect(Ray& ray, float& majorant_mu_t) const;

	Build_node root_;

	int3 dimensions_;

private:

	bool intersect(Ray& ray, Build_node const* node, Box const& aabb,
				   float& majorant_mu_t) const;
};

}}

#endif
