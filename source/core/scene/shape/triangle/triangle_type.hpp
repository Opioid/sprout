#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_TYPE_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_TYPE_HPP

namespace scene::shape::triangle {

using Triangle_type = struct Triangle_MTE;

using Intersection_triangle_type = struct Intersection_triangle_MTE;
using Shading_triangle_type      = struct Shading_triangle_MTE;

// using Shading_vertex_type = struct Shading_vertex_MT;
using Shading_vertex_type = struct Shading_vertex_MTC;
// using Shading_vertex_type = struct Shading_vertex_MTCC;

using Vertex_type = struct Vertex_MTC;

}  // namespace scene::shape::triangle

#endif
