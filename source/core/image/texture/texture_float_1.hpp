#pragma once

#include "texture.hpp"
#include "image/typed_image.hpp"

namespace image { namespace texture {

class Float_1 : public Texture {

public:

	Float_1(std::shared_ptr<Image> image);

	virtual float  at_1(int32_t x, int32_t y) const final override;
	virtual float2 at_2(int32_t x, int32_t y) const final override;
	virtual float3 at_3(int32_t x, int32_t y) const final override;
	virtual float4 at_4(int32_t x, int32_t y) const final override;

	virtual float4 gather_1(int4 xy_xy1) const final override;
	virtual void   gather_2(int4 xy_xy1, float2 c[4]) const final override;
	virtual void   gather_3(int4 xy_xy1, float3 c[4]) const final override;

	virtual float  at_element_1(int32_t x, int32_t y, int32_t element) const final override;
	virtual float2 at_element_2(int32_t x, int32_t y, int32_t element) const final override;
	virtual float3 at_element_3(int32_t x, int32_t y, int32_t element) const final override;
	virtual float4 at_element_4(int32_t x, int32_t y, int32_t element) const final override;

	virtual float  at_1(int32_t x, int32_t y, int32_t z) const final override;
	virtual float2 at_2(int32_t x, int32_t y, int32_t z) const final override;
	virtual float3 at_3(int32_t x, int32_t y, int32_t z) const final override;
	virtual float4 at_4(int32_t x, int32_t y, int32_t z) const final override;

private:

	const image::Float_1& image_;
};

}}
