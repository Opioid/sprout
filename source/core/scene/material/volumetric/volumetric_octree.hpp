#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"

namespace image::texture { class Texture; }

namespace scene {

struct Ray;

namespace material::volumetric {

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

	Node* allocate_nodes(uint32_t num_nodes);

	void set_dimensions(int3 const& dimensions);

	bool intersect(Ray& ray, float& majorant_mu_t) const;

private:

	bool intersect(Ray& ray, uint32_t node_id, Box const& aabb, float& majorant_mu_t) const;

	uint32_t num_nodes_;
	Node*    nodes_;

	int3 dimensions_;

	float3 inv_2_dimensions_;
};

}}

#endif
