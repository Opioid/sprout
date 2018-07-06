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
    int32_t children;

    float majorant_mu_t;
};

class Gridtree {
  public:
    Gridtree();
    ~Gridtree();

    Node* allocate_nodes(int32_t num_nodes);

    void set_dimensions(int3 const& dimensions, int3 const& cell_dimensions, int3 const& num_cells);

    bool is_valid() const;

    bool intersect(math::Ray& ray, float& majorant_mu_t) const;

  private:
    int32_t num_nodes_;
    Node*   nodes_;

    int3 dimensions_;
    int3 num_cells_;

    float3 cell_dimensions_;

    float3 factor_;
};

class Octree {
  public:
    Octree();
    ~Octree();

    Node* allocate_nodes(int32_t num_nodes);

    void set_dimensions(int3 const& dimensions);

    bool is_valid() const;

    bool intersect(math::Ray& ray, float& majorant_mu_t) const;

    bool intersect_f(math::Ray& ray, float& majorant_mu_t) const;

  private:
    bool intersect(math::Ray& ray, int32_t node_id, Box const& box, float& majorant_mu_t) const;

    void intersect_children(math::Ray& ray, Node const& node, Box const& box,
                            float& majorant_mu_t) const;

    int32_t num_nodes_;
    Node*   nodes_;

    int3 dimensions_;

    float3 inv_2_dimensions_;

  public:
    Gridtree gridtree_;
};

}  // namespace scene::material::volumetric

#endif
