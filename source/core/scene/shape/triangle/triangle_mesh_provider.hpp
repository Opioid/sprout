#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PROVIDER_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PROVIDER_HPP

#include "resource/resource_provider.hpp"

#include <string>
#include <vector>

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

    Provider();

    ~Provider() override;

    Shape* load(std::string const& filename, Variants const& options, Resources& resources,
                std::string& resolved_name) final;

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
                resource::Manager& resources) final;

  private:
    Shape* load_morphable_mesh(std::string const& filename, Strings const& morph_targets,
                               Resources& resources);

    static Shape* create_mesh(Triangles& triangles, Vertices& vertices, uint32_t num_parts,
                              thread::Pool& threads);

    static void build_bvh(Mesh& mesh, uint32_t num_triangles, Index_triangle const* const triangles,
                          Vertex_stream const& vertices, thread::Pool& threads);

    //	static void build_bvh(Mesh& mesh, Triangles const& triangles, Vertices const& vertices,
    //						  BVH_preset bvh_preset, thread::Pool& threads);

    static Shape* load_binary(std::istream& stream, thread::Pool& threads);
};

}  // namespace triangle
}  // namespace scene::shape

#endif
