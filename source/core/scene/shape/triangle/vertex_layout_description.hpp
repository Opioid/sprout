#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_LAYOUT_DESCRIPTION_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_LAYOUT_DESCRIPTION_HPP

#include <iosfwd>
#include <string>

namespace scene::shape::triangle {

class Vertex_layout_description {
  public:
    enum class Encoding { UInt16, UInt32, Float32, Float32x2, Float32x3 };

    struct Element {
        std::string semantic_name;
        uint32_t    semantic_index = 0;
        Encoding    encoding;
        uint32_t    stream      = 0;
        uint32_t    byte_offset = 0;
    };
};

}  // namespace scene::shape::triangle

/*
std::stringstream& operator<<(std::stringstream& stream,
                                                          scene::shape::triangle::Vertex_layout_description::Encoding
encoding);
                                                          */

std::stringstream& operator<<(
    std::stringstream&                                                stream,
    const scene::shape::triangle::Vertex_layout_description::Element& element);

#endif
