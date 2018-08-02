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

struct Node {
    bool has_children() const noexcept {
        return 0 != is_parent;
    }

    uint32_t index() const noexcept {
        return children_or_data;
    }

    void set_children(uint32_t index) noexcept {
        is_parent        = 1;
        children_or_data = index;
    }

    void set_data(uint32_t index) noexcept {
        is_parent        = 0;
        children_or_data = index;
    }

    uint32_t is_parent : 1;
    uint32_t children_or_data : 31;
};

class Gridtree {
  public:
    Gridtree() noexcept;

    ~Gridtree() noexcept;

    Node* allocate_nodes(uint32_t num_nodes) noexcept;

    CM* allocate_data(uint32_t num_data) noexcept;

    void set_dimensions(int3 const& dimensions, int3 const& cell_dimensions,
                        int3 const& num_cells) noexcept;

    bool is_valid() const noexcept;

    bool intersect(math::Ray& ray, CM& data) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    uint32_t num_nodes_;
    Node*    nodes_;

    uint32_t num_data_;
    CM*      data_;

    int3 dimensions_;
    int3 num_cells_;

    float3 cell_dimensions_;

    float3 factor_;
};

}  // namespace volumetric
}  // namespace scene::material

#endif
