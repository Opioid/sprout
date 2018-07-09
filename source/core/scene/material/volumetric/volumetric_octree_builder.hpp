#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_BUILDER_HPP

#include "base/math/vector2.hpp"
#include "volumetric_octree.hpp"

namespace image::texture {
class Texture;
}

namespace scene::material::volumetric {

class Octree_builder {
  public:
    using Texture = image::texture::Texture;

    void build(Gridtree& tree, Texture const& texture, float2 min_max_extinction);

  private:
    struct Build_node {
        ~Build_node();

        Build_node* children[8];

        Node::Data data;
    };

    void split(Build_node* node, Box const& box, Texture const& texture, float2 min_max_extinction,
               uint32_t depth, uint32_t max_depth);

    void serialize(Build_node* node, int32_t current, int32_t& next);

    int32_t num_nodes_;

    Node* nodes_;
};

}  // namespace scene::material::volumetric

#endif
