#ifndef SU_EXTENSION_PROCEDURAL_MESH_GRASS_HPP
#define SU_EXTENSION_PROCEDURAL_MESH_GRASS_HPP

#include <vector>
#include "base/math/vector3.hpp"
#include "core/scene/shape/triangle/triangle_mesh_generator.hpp"

namespace scene::shape {

namespace triangle {
struct Index_triangle;
}

struct Vertex;

}  // namespace scene::shape

namespace procedural::mesh {

class Grass : public scene::shape::triangle::Generator {
  public:
    using Shape = scene::shape::Shape;

    virtual std::shared_ptr<Shape> create_mesh(json::Value const& mesh_value,
                                               resource::Manager& manager) override final;

  private:
    void add_blade(float3 const& offset, float rotation_y, float lean_factor, float width,
                   float height, uint32_t vertex_offset,
                   std::vector<scene::shape::triangle::Index_triangle>& triangles,
                   std::vector<scene::shape::Vertex>&                   vertices) const;

    static void calculate_normals(std::vector<scene::shape::triangle::Index_triangle>& triangles,
                                  std::vector<scene::shape::Vertex>&                   vertices);
};

}  // namespace procedural::mesh

#endif
