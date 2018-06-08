#include "image.hpp"
#include "base/math/vector3.inl"

namespace image {

Image::Description::Description(Type type, int2 dimensions, int32_t num_elements)
    : type(type), dimensions(dimensions, 1), num_elements(num_elements) {}

Image::Description::Description(Type type, int3 const& dimensions, int32_t num_elements)
    : type(type), dimensions(dimensions), num_elements(num_elements) {}

size_t Image::Description::num_pixels() const {
    return static_cast<size_t>(dimensions[0] * dimensions[1] * dimensions[2] * num_elements);
}

uint32_t Image::Description::num_channels() const {
    switch (type) {
        case Type::Byte1:
        case Type::Float1:
            return 1;
        case Type::Byte2:
        case Type::Float2:
            return 2;
        case Type::Byte3:
        case Type::Float3:
            return 3;
        case Type::Float4:
            return 4;
        default:
            return 0;
    }
}

Image::Image(Description const& description)
    : description_(description),
      area_(description.dimensions[0] * description.dimensions[1]),
      volume_(description.dimensions[0] * description.dimensions[1] * description.dimensions[2]) {}

Image::~Image() {}

const Image::Description& Image::description() const {
    return description_;
}

int2 Image::dimensions2() const {
    return description_.dimensions.xy();
}

int32_t Image::area() const {
    return area_;
}

int32_t Image::volume() const {
    return volume_;
}

int2 Image::coordinates_2(int32_t index) const {
    int2 c;
    c[1] = index / description_.dimensions[0];
    c[0] = index - c[1] * description_.dimensions[0];
    return c;
}

void Image::resize(Description const& description) {
    description_ = description;
    area_        = description.dimensions[0] * description.dimensions[1];
    volume_ = description.dimensions[0] * description.dimensions[1] * description.dimensions[2];
}

}  // namespace image
