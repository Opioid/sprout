#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP

#include "base/math/vector3.hpp"

namespace image::texture { class Texture; }

namespace scene::material::volumetric {

class Octree_builder {

public:

	void build(image::texture::Texture const& texture);

private:

	struct Box {
		Box() = default;
		Box(int3 const& min, int3 const& max);

		int3 bounds[2];
	};

	void split(image::texture::Texture const& texture, Box const& box, uint32_t depth);
};

}

#endif
