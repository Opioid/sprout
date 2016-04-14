#pragma once

#include "image.hpp"
#include "base/color/color.hpp"
#include "base/memory/tiled_array.hpp"

namespace image {

template<typename T, uint32_t Log_tile_size>
class Tiled_image : public Image {
public:

	Tiled_image(const Image::Description& description);
	~Tiled_image();

	const T& at(uint32_t x, uint32_t y) const;
	T& at(uint32_t x, uint32_t y);

private:

	memory::Tiled_array<T, Log_tile_size> data_;
};

// using Image_byte_3 = Tiled_image<color::Color3c, 1> Image_byte_3;

}
