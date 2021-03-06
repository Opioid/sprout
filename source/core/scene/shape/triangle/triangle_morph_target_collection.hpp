#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MORPH_TARGET_COLLECTION_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MORPH_TARGET_COLLECTION_HPP

#include <cstdint>
#include <vector>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

namespace animation {
struct Morphing;
}

namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;

class Morph_target_collection {
  public:
    using Morphing = animation::Morphing;

    Morph_target_collection();

    Morph_target_collection(Morph_target_collection&& other);

    std::vector<Index_triangle> const& triangles() const;

    std::vector<Index_triangle>& triangles();

    uint32_t num_vertices() const;

    void add_swap_vertices(std::vector<Vertex>& vertices);

    void morph(Morphing const* morphings, uint32_t num_frames, Threads& threads, Vertex* vertices);

  private:
    void morph(Morphing const& morphing, Threads& threads, Vertex* vertices);

    std::vector<Index_triangle> triangles_;

    std::vector<std::vector<Vertex>> morph_targets_;
};

}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
