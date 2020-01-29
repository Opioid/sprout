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
class Vertex_stream;

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

    Shape* load(std::string const& filename, Variants const& options, Resources& resources,
                std::string& resolved_name) noexcept override final;

    struct Description {
        uint32_t num_triangles;
        uint32_t num_vertices;
        uint32_t positions_stride;
        uint32_t normals_stride;
        uint32_t tangents_stride;
        uint32_t uvs_stride;
        uint32_t num_parts;

        float const* positions;
        float const* normals;
        float const* tangents;
        float const* uvs;

        uint32_t const* indices;
        uint32_t const* parts;
    };

    Shape* load(void const* data, std::string const& source_name, Variants const& options,
                resource::Manager& resources) noexcept override final;

    size_t num_bytes() const noexcept override final;

    size_t num_bytes(Shape const* resource) const noexcept override final;

    static Shape* create_mesh(Triangles const& triangles, Vertices const& vertices,
                              uint32_t num_parts, thread::Pool& threads) noexcept;

  private:
    Shape* load_morphable_mesh(std::string const& filename, Strings const& morph_targets,
                               Resources& resources) noexcept;

    static void build_bvh(Mesh& mesh, uint32_t num_triangles, Index_triangle const* const triangles,
                          Vertex_stream const& vertices, thread::Pool& threads) noexcept;

    //	static void build_bvh(Mesh& mesh, Triangles const& triangles, Vertices const& vertices,
    //						  BVH_preset bvh_preset, thread::Pool& threads);

    static Shape* load_binary(std::istream& stream, thread::Pool& threads) noexcept;
};

}  // namespace triangle
}  // namespace scene::shape

#endif
