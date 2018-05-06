#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP

namespace image::texture { class Texture; }

namespace scene::material::volumetric {

class Octree_builder {

public:

	void build(image::texture::Texture const& texture);
};

}

#endif
