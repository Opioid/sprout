#pragma once

#include "texture_2d.hpp"
#include "image/typed_image.hpp"

namespace image { namespace texture {

class Texture_2D_float_3 : public Texture_2D {
public:

	Texture_2D_float_3(std::shared_ptr<Image> image);

	virtual float        at_1(int32_t x, int32_t y) const final override;
	virtual math::float2 at_2(int32_t x, int32_t y) const final override;
	virtual math::float3 at_3(int32_t x, int32_t y) const final override;
	virtual math::float4 at_4(int32_t x, int32_t y) const final override;

private:

	const Image_float_3& image_;
};

}}
