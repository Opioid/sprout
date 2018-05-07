#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP

#include "volumetric_octree.hpp"
#include "base/math/vector3.hpp"

namespace image::texture { class Texture; }

namespace scene::material::volumetric {

class Octree_builder {

public:

	void build(Octree& tree, image::texture::Texture const& texture, float max_extinction);

private:

	void split(Build_node* node, Box const& box, image::texture::Texture const& texture,
			   float max_extinction, uint32_t depth);
};

}

#endif
