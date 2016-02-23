#pragma once

#include "texture_2d.hpp"
#include "image/typed_image.hpp"

namespace image { namespace texture {

class Texture_2D_byte_3_snorm : public Texture_2D {
public:

	Texture_2D_byte_3_snorm(std::shared_ptr<Image> image);

	virtual float        at_1(int32_t x, int32_t y) const final override;
	virtual math::float2 at_2(int32_t x, int32_t y) const final override;
	virtual math::vec3 at_3(int32_t x, int32_t y) const final override;
	virtual math::float4 at_4(int32_t x, int32_t y) const final override;

	virtual float        at_1(int32_t x, int32_t y, int32_t element) const final override;
	virtual math::float2 at_2(int32_t x, int32_t y, int32_t element) const final override;
	virtual math::vec3 at_3(int32_t x, int32_t y, int32_t element) const final override;
	virtual math::float4 at_4(int32_t x, int32_t y, int32_t element) const final override;

private:

	const Image_byte_3& image_;
};

}}
