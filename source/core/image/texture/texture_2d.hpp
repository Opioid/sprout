#pragma once

#include "base/math/vector.hpp"
#include <memory>

namespace image {

class Image;

namespace texture {

class Texture_2D {
public:

	Texture_2D(std::shared_ptr<Image> image);
	virtual ~Texture_2D();

	const Image* image() const;

	int2 dimensions() const;
	float2 dimensions_float() const;

	int32_t num_elements() const;

	virtual float  at_1(int32_t x, int32_t y) const = 0;
	virtual float2 at_2(int32_t x, int32_t y) const = 0;
	virtual float3 at_3(int32_t x, int32_t y) const = 0;
	virtual float4 at_4(int32_t x, int32_t y) const = 0;

	virtual float  at_1(int32_t x, int32_t y, int32_t element) const = 0;
	virtual float2 at_2(int32_t x, int32_t y, int32_t element) const = 0;
	virtual float3 at_3(int32_t x, int32_t y, int32_t element) const = 0;
	virtual float4 at_4(int32_t x, int32_t y, int32_t element) const = 0;

	float3 average_3() const;
	float3 average_3(int32_t element) const;

	float4 average_4() const;

protected:

	std::shared_ptr<Image> image_;

	float2 dimensions_float_;
};

}}
