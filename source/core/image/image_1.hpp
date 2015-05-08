#pragma once

#include "image.hpp"

namespace image {

class Image_1 : public Image {
public:

	Image_1(const Description& description);
	virtual ~Image_1();

	virtual float        at1(uint32_t index) const final override;
	virtual math::float3 at3(uint32_t index) const final override;
	virtual math::float4 at4(uint32_t index) const final override;

	virtual void set1(uint32_t index, float value) final override;
	virtual void set3(uint32_t index, const math::float3& value) final override;
	virtual void set4(uint32_t index, const math::float4& value) final override;

private:

	float* data_;
};

}

