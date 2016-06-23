#pragma once

#include "texture_2d.hpp"
#include "image/typed_image.hpp"

namespace image { namespace texture {

class Texture_2D_byte_2_snorm : public Texture_2D {
public:

	Texture_2D_byte_2_snorm(std::shared_ptr<Image> image);

	virtual float        at_1(int32_t x, int32_t y) const final override;
	virtual float2 at_2(int32_t x, int32_t y) const final override;
	virtual float3 at_3(int32_t x, int32_t y) const final override;
	virtual float4 at_4(int32_t x, int32_t y) const final override;

	virtual float        at_1(int32_t x, int32_t y, int32_t element) const final override;
	virtual float2 at_2(int32_t x, int32_t y, int32_t element) const final override;
	virtual float3 at_3(int32_t x, int32_t y, int32_t element) const final override;
	virtual float4 at_4(int32_t x, int32_t y, int32_t element) const final override;

private:

	const Image_byte_2& image_;
};

}}
