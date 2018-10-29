#pragma once

#include "base/memory/tiled_array.inl"
#include "tiled_image.hpp"

namespace image {

template <typename T, uint32_t Log_tile_size>
Tiled_image<T, Log_tile_size>::Tiled_image(const Image::Description& description)
    : Image(description), data_(description.dimensions[0], description.dimensions[1]) {}

template <typename T, uint32_t Log_tile_size>
Tiled_image<T, Log_tile_size>::~Tiled_image() {}

template <typename T, uint32_t Log_tile_size>
T const& Tiled_image<T, Log_tile_size>::at(uint32_t x, uint32_t y) const {
    return data_.at(x, y);
}

template <typename T, uint32_t Log_tile_size>
T& Tiled_image<T, Log_tile_size>::at(uint32_t x, uint32_t y) {
    return data_.at(x, y);
}

}  // namespace image
