#pragma once

#include "image/typed_image.hpp"

namespace image::encoding {

class Srgb {

public:

	Srgb(int2 dimensions);
	~Srgb();

	const byte3* data() const;

	void to_sRGB(const image::Float3& image, int32_t begin, int32_t end);
	void to_sRGB(const image::Float4& image, int32_t begin, int32_t end);

	void to_byte(const image::Float3& image, int32_t begin, int32_t end);
	void to_byte(const image::Float4& image, int32_t begin, int32_t end);

protected:

	byte3* rgb_;
};

class Srgb_alpha {

public:

	Srgb_alpha(int2 dimensions);
	~Srgb_alpha();

	const byte4* data() const;

	void to_sRGB(const image::Float3& image, int32_t begin, int32_t end);
	void to_sRGB(const image::Float4& image, int32_t begin, int32_t end);

	void to_byte(const image::Float3& image, int32_t begin, int32_t end);
	void to_byte(const image::Float4& image, int32_t begin, int32_t end);

protected:

	byte4* rgba_;
};

}
