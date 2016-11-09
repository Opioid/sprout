#pragma once

#include "image/typed_image.hpp"

namespace image { namespace procedural {

template<typename T>
class Typed_renderer {

public:

	Typed_renderer(int2 dimensions, int32_t sqrt_num_samples = 1);
	~Typed_renderer();

	void set_brush(T color);

	void clear();

	void draw_circle(float2 pos, float radius);

	void resolve(Typed_image<T>& target) const;

private:

	void set_sample(int32_t x, int32_t y, T color);
	void set_row(int32_t start_x, int32_t end_x, int32_t y, T color);

	int32_t sqrt_num_samples_;

	int2 dimensions_;

	float2 dimensions_f_;

	T* samples_;

	T brush_;
};

class Renderer {

public:

	Renderer(int2 dimensions, int32_t sqrt_num_samples = 1);
	~Renderer();

	void set_brush(float3_p color);
	void set_brush(const float4& color);

	void clear();

	void draw_circle(float2 pos, float radius);

	void resolve_sRGB(Image_byte_3& target) const;
	void resolve(Image_byte_3& target) const;

	void resolve(Image_byte_1& target) const;

private:

	void set_sample(int32_t x, int32_t y, const float4& color);
	void set_row(int32_t start_x, int32_t end_x, int32_t y, const float4& color);

	int32_t sqrt_num_samples_;

	int2 dimensions_;

	float2 dimensions_f_;

	float4* samples_;

	float4 brush_;
};

}}
