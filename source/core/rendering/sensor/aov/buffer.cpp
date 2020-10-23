#include "buffer.hpp"
#include "base/math/vector4.inl"

namespace rendering::sensor::aov {

Buffer::Buffer() : len_(0), shading_normal_(nullptr) {}

Buffer::~Buffer() {
    delete[] shading_normal_;
}

void Buffer::resize(int2 dimensions) {
    uint32_t const len = uint32_t(dimensions[0]) * uint32_t(dimensions[1]);

    if (len_ != len) {
        len_ = len;

        delete[] shading_normal_;

        shading_normal_ = new float3[len];
    }
}

void Buffer::clear() {
    for (uint32_t i = 0, len = len_; i < len; ++i) {
        shading_normal_[i] = float3(0.f);
    }
}

void Buffer::add_pixel(int32_t id, float4 const& value, float weight, Property aov) {
    if (Property::Shading_normal == aov) {
        shading_normal_[id] += weight * value.xyz();
    }
}

float4 Buffer::value(int32_t id, Property aov) const {
    if (Property::Shading_normal == aov) {
        return float4(shading_normal_[id], 0.f);
    }

    return float4(0.f);
}

}
