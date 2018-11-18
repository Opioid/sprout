#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP

#include "base/math/vector2.hpp"
#include "scene/material/collision_coefficients.hpp"
#include "volumetric_octree.hpp"

namespace image::texture {
class Texture;
}

namespace scene::material::volumetric {

class Octree_builder {
  public:
    using Texture = image::texture::Texture;

    void build(Gridtree& tree, Texture const& texture, CM const& idata);

  private:
    struct Build_node {
        ~Build_node();

        Build_node* children[8];

        CM data;
    };

    void split(Build_node* node, Box const& box, Texture const& texture, CM const& idata,
               uint32_t depth);

    void serialize(Build_node* node, uint32_t current, uint32_t& next, uint32_t& data);

    uint32_t num_nodes_;
    Node*    nodes_;

    uint32_t num_data_;
    CM*      data_;

    uint32_t num_homogeneous_non_empty_;
};

}  // namespace scene::material::volumetric

#endif
