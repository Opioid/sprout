#pragma once

#include "image/typed_image.hpp"
#include "base/color/color.hpp"
#include "base/math/vector.hpp"

namespace image { namespace encoding {

class Srgb {
public:

	Srgb(math::int2 dimensions);
	virtual ~Srgb();

protected:

	void to_sRGB(const image::Image_float_4& image, int32_t begin, int32_t end);

	void to_byte(const image::Image_float_4& image, int32_t begin, int32_t end);


	color::Color4c* rgba_;
};

}}
