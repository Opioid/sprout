#ifndef SU_CORE_IMAGE_PROCEDURAL_RENDERER_HPP
#define SU_CORE_IMAGE_PROCEDURAL_RENDERER_HPP

#include "image/typed_image.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector4.hpp"

namespace image::procedural {

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

	void set_brush(const float3& color);
	void set_brush(const float4& color);

	void clear();

	void draw_circle(float2 pos, float radius);

	void resolve_sRGB(Byte3& target) const;
	void resolve(Byte3& target) const;

	void resolve(Byte1& target) const;

private:

	void set_sample(int32_t x, int32_t y, const float4& color);
	void set_row(int32_t start_x, int32_t end_x, int32_t y, const float4& color);

	int32_t sqrt_num_samples_;

	int2 dimensions_;

	float2 dimensions_f_;

	float4* samples_;

	float4 brush_;
};

}

#endif
