#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MORPH_TARGET_COLLECTION_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MORPH_TARGET_COLLECTION_HPP

#include <cstdint>
#include <vector>

namespace thread {
class Pool;
}

namespace scene::shape {

struct Vertex;

namespace triangle {

struct Index_triangle;

class Morph_target_collection {
  public:
    std::vector<Index_triangle>&       triangles();
    const std::vector<Index_triangle>& triangles() const;

    const std::vector<Vertex>& vertices(uint32_t target) const;

    void add_swap_vertices(std::vector<Vertex>& vertices);

    void morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool,
               std::vector<Vertex>& vertices);

  private:
    std::vector<Index_triangle> triangles_;

    std::vector<std::vector<Vertex>> morph_targets_;
};

}  // namespace triangle
}  // namespace scene::shape

#endif
