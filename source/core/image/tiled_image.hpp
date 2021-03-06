#pragma once

#include "base/memory/tiled_array.hpp"
#include "base/spectrum/rgb.hpp"
#include "image.hpp"

namespace image {

template <typename T, uint32_t Log_tile_size>
class Tiled_image : public Image {
  public:
    Tiled_image(const Description& description);
    ~Tiled_image();

    T const& at(uint32_t x, uint32_t y) const;
    T&       at(uint32_t x, uint32_t y);

  private:
    memory::Tiled_array<T, Log_tile_size> data_;
};

// using Byte3 = Tiled_image<spectrum::Color3c, 1> Byte3;

}  // namespace image
