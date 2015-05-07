#pragma once

#include "image.hpp"

namespace image {

class Image_4 : public Image {
public:

	Image_4(const Description& description);
	virtual ~Image_4();

	virtual math::float3 at3(uint32_t index) const;
	virtual math::float4 at4(uint32_t index) const;

	virtual void set3(uint32_t index, const math::float3& value);
	virtual void set4(uint32_t index, const math::float4& value);

private:

	math::float4* data_;
};

}
