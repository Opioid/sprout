#pragma once

#include "image/typed_image.hpp"

namespace image { namespace procedural {

class Renderer {

public:

	Renderer(int2 dimensions, int num_sqrt_samples = 1);
	~Renderer();

	void set_brush(float3_p color);
	void set_brush(const float4& color);

	void clear();

	void draw_circle(float2 pos, float radius);

	void resolve(Image_byte_3& target) const;

	void resolve(Image_byte_1& target) const;

private:

	void set_sample(int x, int y, const float4& color);
	void set_row(int start_x, int end_x, int y, const float4& color);

	int num_sqrt_samples_;

	int2 dimensions_;

	float2 dimensions_f_;

	float4* samples_;

	float4 brush_;
};

}}
