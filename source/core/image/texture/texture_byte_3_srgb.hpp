#ifndef SU_CORE_IMAGE_TEXTURE_BYTE3_SRGB_HPP
#define SU_CORE_IMAGE_TEXTURE_BYTE3_SRGB_HPP

#include "texture.hpp"
#include "image/typed_image.hpp"

namespace image::texture {

class Byte3_sRGB final : public Texture {

public:

	Byte3_sRGB(const std::shared_ptr<Image>& image);

	virtual float3 at_3(int32_t i) const override final;

	virtual float  at_1(int32_t x, int32_t y) const override final;
	virtual float2 at_2(int32_t x, int32_t y) const override final;
	virtual float3 at_3(int32_t x, int32_t y) const override final;
	virtual float4 at_4(int32_t x, int32_t y) const override final;

	virtual float4 gather_1(const int4& xy_xy1) const override final;
	virtual void   gather_2(const int4& xy_xy1, float2 c[4]) const override final;
	virtual void   gather_3(const int4& xy_xy1, float3 c[4]) const override final;

	virtual float  at_element_1(int32_t x, int32_t y, int32_t element) const override final;
	virtual float2 at_element_2(int32_t x, int32_t y, int32_t element) const override final;
	virtual float3 at_element_3(int32_t x, int32_t y, int32_t element) const override final;
	virtual float4 at_element_4(int32_t x, int32_t y, int32_t element) const override final;

	virtual float  at_1(int32_t x, int32_t y, int32_t z) const override final;
	virtual float2 at_2(int32_t x, int32_t y, int32_t z) const override final;
	virtual float3 at_3(int32_t x, int32_t y, int32_t z) const override final;
	virtual float4 at_4(int32_t x, int32_t y, int32_t z) const override final;

private:

	const Byte3& image_;
};

}

#endif
