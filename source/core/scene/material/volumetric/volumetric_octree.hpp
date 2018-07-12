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

struct Node {
    static uint32_t constexpr Children_mask = 0x7FFFFFFFu;

    uint32_t children_or_data;

    bool has_children() const {
        return 0 == (children_or_data & ~Children_mask);
    }

    uint32_t index() const {
        return children_or_data & Children_mask;
    }

    void set_children(uint32_t index) {
        children_or_data = index;
    }

    void set_data(uint32_t index) {
        children_or_data = index | ~Children_mask;
    }
};

struct Control_data {
    float minorant_mu_a;
    float minorant_mu_s;
    float minorant_mu_t;
    float majorant_mu_t;
};

class Gridtree {
  public:
    Gridtree();
    ~Gridtree();

    Node* allocate_nodes(uint32_t num_nodes);

    Control_data* allocate_data(uint32_t num_data);

    void set_dimensions(int3 const& dimensions, int3 const& cell_dimensions, int3 const& num_cells);

    bool is_valid() const;

    bool intersect(math::Ray& ray, Control_data& data) const;

    size_t num_bytes() const;

  private:
    uint32_t num_nodes_;
    Node*    nodes_;

    uint32_t      num_data_;
    Control_data* data_;

    int3 dimensions_;
    int3 num_cells_;

    float3 cell_dimensions_;

    float3 factor_;
};

}  // namespace scene::material::volumetric

#endif
