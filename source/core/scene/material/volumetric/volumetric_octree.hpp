#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"

namespace math { struct Ray; }

namespace image::texture { class Texture; }

namespace scene::material::volumetric {

struct Box {
	Box() = default;
	Box(int3 const& min, int3 const& max);

	int3 bounds[2];
};

class Octree {

public:

	struct Node {
		uint32_t children;

		float majorant_mu_t;
	};

	Octree();
	~Octree();

	Node* allocate_nodes(uint32_t num_nodes, uint32_t deepest_uniform_level);

	void set_dimensions(int3 const& dimensions);

	bool is_valid() const;

	bool intersect(math::Ray& ray, float& majorant_mu_t) const;

private:

	bool intersect(math::Ray& ray, uint32_t node_id, Box const& box, float& majorant_mu_t) const;

	void intersect_children(math::Ray& ray, Node const& node, Box const& box,
							float& majorant_mu_t) const;

	uint32_t num_nodes_;
	uint32_t deepest_uniform_level_;
	Node*    nodes_;

	int3 dimensions_;

	float3 inv_2_dimensions_;
};

}

#endif
