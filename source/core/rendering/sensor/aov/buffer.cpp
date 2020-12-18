#include "buffer.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector4.inl"

namespace rendering::sensor::aov {

Buffer::Buffer() : buffers_len_(0), buffer_len_(0), buffers_(nullptr) {}

Buffer::~Buffer() {
    for (uint32_t i = 0, len = buffers_len_; i < len; ++i) {
        delete[] buffers_[i];
    }

    delete[] buffers_;
}

void Buffer::resize(int2 dimensions, Value_pool const& aovs) {
    uint32_t const buffers_len = aovs.num_slots();
    uint32_t const buffer_len  = uint32_t(dimensions[0]) * uint32_t(dimensions[1]);

    if (buffers_len_ == buffers_len && buffer_len_ == buffer_len) {
        return;
    }

    if (buffers_len_ != buffers_len) {
        for (uint32_t i = 0, len = buffers_len_; i < len; ++i) {
            delete[] buffers_[i];
        }

        delete[] buffers_;

        buffer_len_ = 0;

        buffers_ = new float4* [buffers_len] { nullptr };

        buffers_len_ = buffers_len;
    }

    if (buffer_len_ != buffer_len) {
        for (uint32_t i = 0, len = buffers_len_; i < len; ++i) {
            delete[] buffers_[i];

            buffers_[i] = new float4[buffer_len];
        }

        buffer_len_ = buffer_len;
    }
}

void Buffer::clear(Value_pool const& aovs) {
    for (uint32_t i = 0, len = buffers_len_; i < len; ++i) {
        Operation const op = operation(aovs.property(i));

        float const iv = initial_value(op);

        float4* buffer = buffers_[i];
        for (uint32_t j = 0, jlen = buffer_len_; j < jlen; ++j) {
            buffer[j] = float4(iv);
        }
    }
}

void Buffer::add_pixel(int32_t id, uint32_t slot, float4_p value, float weight) {
    buffers_[slot][id] += weight * value;
}

void Buffer::add_pixel_atomic(int32_t id, uint32_t slot, float4_p value, float weight) {
    float4* buffer = buffers_[slot];

    auto& target = buffer[id];
    atomic::add_assign(target[0], weight * value[0]);
    atomic::add_assign(target[1], weight * value[1]);
    atomic::add_assign(target[2], weight * value[2]);
}

void Buffer::overwrite_pixel(int32_t id, uint32_t slot, float4_p value) {
    buffers_[slot][id] = value;
}

float4 Buffer::value(int32_t id, uint32_t slot) const {
    return buffers_[slot][id];
}

}  // namespace rendering::sensor::aov
