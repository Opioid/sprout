#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_OCTREE_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"

namespace math {
struct Ray;
}

namespace image::texture {
class Texture;
}

namespace scene::material::volumetric {

struct Box {
    int3 bounds[2];
};

struct alignas(16) Node {
    int32_t children;

    struct Data {
        float minorant_mu_t;
        float majorant_mu_t;
        float min_density;
    };

    Data data;
};

class Gridtree {
  public:
    Gridtree();
    ~Gridtree();

    Node* allocate_nodes(int32_t num_nodes);

    void set_dimensions(int3 const& dimensions, int3 const& cell_dimensions, int3 const& num_cells);

    bool is_valid() const;

    bool intersect(math::Ray& ray, Node::Data& data) const;

  private:
    int32_t num_nodes_;
    Node*   nodes_;

    int3 dimensions_;
    int3 num_cells_;

    float3 cell_dimensions_;

    float3 factor_;
};

}  // namespace scene::material::volumetric

#endif
