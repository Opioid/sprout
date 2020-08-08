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

namespace scene::material {

struct CM;

namespace volumetric {

struct Box {
    int3 bounds[2];
};

struct alignas(8) Node {
    bool is_parent() const {
        return 0 != has_children;
    }

    bool is_empty() const {
        return 0x7FFFFFFF == children_or_data;
    }

    uint32_t index() const {
        return children_or_data;
    }

    void set_children(uint32_t index) {
        has_children     = 1;
        children_or_data = index;
    }

    void set_data(uint32_t index) {
        has_children     = 0;
        children_or_data = index;
    }

    void set_empty() {
        has_children     = 0;
        children_or_data = 0x7FFFFFFF;
    }

    uint32_t has_children : 1;
    uint32_t children_or_data : 31;
};

class alignas(64) Gridtree {
  public:
    Gridtree();

    ~Gridtree();

    Node* allocate_nodes(uint32_t num_nodes);

    CM* allocate_data(uint32_t num_data);

    void set_dimensions(int3 const& dimensions, int3 const& num_cells);

    bool is_valid() const;

    bool intersect(ray& ray, CM& data) const;

    static int32_t constexpr Log2_cell_dim = 5;
    static int32_t constexpr Cell_dim      = 1 << Log2_cell_dim;

  private:
    uint32_t num_nodes_;
    uint32_t num_data_;

    Node* nodes_;
    CM*   data_;

    int3  dimensions_;
    uint3 num_cells_;

    float3 inv_dimensions_;
};

}  // namespace volumetric
}  // namespace scene::material

#endif
