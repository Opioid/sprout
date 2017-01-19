#pragma once

namespace scene { namespace shape { namespace triangle {

using Triangle_type = struct Triangle_MTE;

using Intersection_triangle_type = struct Intersection_triangle_MTE;
using Shading_triangle_type = struct Shading_triangle_MTE;

using Intersection_vertex_type = struct Intersection_vertex_MT;
// using Shading_vertex_type = struct Shading_vertex_MT;
using Shading_vertex_type = struct Shading_vertex_MTC;
// using Shading_vertex_type = struct Shading_vertex_MTCC;

}}}
