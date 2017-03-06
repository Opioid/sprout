#pragma once

#include "image.hpp"
#include "base/math/vector4.hpp"

namespace image {

template<typename T>
class Typed_image : public Image {

public:

	Typed_image();
	Typed_image(const Image::Description& description);
	~Typed_image();

	Typed_image<T> clone() const;

	void resize(const Image::Description& description);

	void clear(T v);

	T load(int32_t index) const;

	void store(int32_t index, T v);

	const T& at(int32_t index) const;
	T& at(int32_t index);

	T load(int32_t x, int32_t y) const;

	void store(int32_t x, int32_t y, T v);

	T load_element(int32_t x, int32_t y, int32_t element) const;

	const T& at(int32_t x, int32_t y) const;
	T& at(int32_t x, int32_t y);

	const T& at_element(int32_t x, int32_t y, int32_t element) const;
	T& at_element(int32_t x, int32_t y, int32_t element);

	T load(int32_t x, int32_t y, int32_t z) const;

	const T& at(int32_t x, int32_t y, int32_t z) const;
	T& at(int32_t x, int32_t y, int32_t z);

	const T& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const;
	T& at_element(int32_t x, int32_t y, int32_t z, int32_t element);

	T* data() const;

	T unsafe_sample(float2 uv, int2 dimensions) const;

	virtual size_t num_bytes() const final override;

private:

	T* data_;
};

using Byte_1 = Typed_image<uint8_t>;
using Byte_2 = Typed_image<byte2>;
using Byte_3 = Typed_image<byte3>;
// using Float_3 = Typed_image<float3>;
using Float_1 = Typed_image<float>;
using Float_2 = Typed_image<float2>;
using Float_3 = Typed_image<packed_float3>;
using Float_4 = Typed_image<float4>;

}
