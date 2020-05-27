#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP

#include "base/math/vector2.hpp"
#include "scene/material/collision_coefficients.hpp"
#include "volumetric_octree.hpp"

namespace image::texture {
class Texture;
}

namespace thread {
class Pool;
}

namespace scene::material::volumetric {

class Octree_builder {
  public:
    using Texture = image::texture::Texture;

    void build(Gridtree& tree, Texture const& texture, CC const* ccs, thread::Pool& threads);

  private:
    struct Build_node {
        ~Build_node();

        Build_node* children[8];

        CM data;
    };

    struct Splitter {
        void split(Build_node* node, Box const& box, Texture const& texture, CC const* ccs,
                   uint32_t depth);

        uint32_t num_nodes = 0;
        uint32_t num_data  = 0;
    };

    void inner_build();

    void serialize(Build_node* node, uint32_t current, uint32_t& next, uint32_t& data);

    Node* nodes_;

    CM* data_;
};

}  // namespace scene::material::volumetric

#endif
