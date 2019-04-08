#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PROVIDER_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PROVIDER_HPP

#include <string>
#include <vector>
#include "resource/resource_provider.hpp"

namespace thread {
class Pool;
}

namespace scene::shape {

class Shape;
struct Vertex;

namespace triangle {

struct Index_triangle;
class Mesh;

class Provider : public resource::Provider<Shape> {
  public:
    using Triangles = std::vector<Index_triangle>;
    using Vertices  = std::vector<Vertex>;
    using Strings   = std::vector<std::string>;

    Provider() noexcept;

    ~Provider() noexcept override;

    Shape* load(std::string const& filename, memory::Variant_map const& options,
                resource::Manager& manager) noexcept override final;

    Shape* load(void const* data, std::string_view mount_folder, memory::Variant_map const& options,
                resource::Manager& manager) noexcept override final;

    size_t num_bytes() const noexcept override final;

    static Shape* create_mesh(Triangles const& triangles, Vertices const& vertices,
                              uint32_t num_parts, thread::Pool& thread_pool) noexcept;

  private:
    Shape* load_morphable_mesh(std::string const& filename, Strings const& morph_targets,
                               resource::Manager& manager) noexcept;

    static void build_bvh(Mesh& mesh, uint32_t num_triangles, Index_triangle const* const triangles,
                          uint32_t num_vertices, Vertex const* const vertices,
                          thread::Pool& thread_pool) noexcept;

    //	static void build_bvh(Mesh& mesh, Triangles const& triangles, Vertices const& vertices,
    //						  BVH_preset bvh_preset, thread::Pool& thread_pool);

    static Shape* load_binary(std::istream& stream, thread::Pool& thread_pool) noexcept;
};

}  // namespace triangle
}  // namespace scene::shape

#endif
