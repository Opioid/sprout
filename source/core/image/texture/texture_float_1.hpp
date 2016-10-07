#pragma once

#include "texture.hpp"
#include "image/typed_image.hpp"

namespace image { namespace texture {

class Texture_float_1 : public Texture {

public:

	Texture_float_1(std::shared_ptr<Image> image);

	virtual float  at_1(int32_t x, int32_t y) const final override;
	virtual float2 at_2(int32_t x, int32_t y) const final override;
	virtual float3 at_3(int32_t x, int32_t y) const final override;
	virtual float4 at_4(int32_t x, int32_t y) const final override;

	virtual float  at_element_1(int32_t x, int32_t y, int32_t element) const final override;
	virtual float2 at_element_2(int32_t x, int32_t y, int32_t element) const final override;
	virtual float3 at_element_3(int32_t x, int32_t y, int32_t element) const final override;
	virtual float4 at_element_4(int32_t x, int32_t y, int32_t element) const final override;

	virtual float  at_1(int32_t x, int32_t y, int32_t z) const final override;
	virtual float2 at_2(int32_t x, int32_t y, int32_t z) const final override;
	virtual float3 at_3(int32_t x, int32_t y, int32_t z) const final override;
	virtual float4 at_4(int32_t x, int32_t y, int32_t z) const final override;

private:

	const Image_float_1& image_;
};

}}
