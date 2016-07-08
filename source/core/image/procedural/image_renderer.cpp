#include "image_renderer.hpp"
#include "image/typed_image.inl"
#include "image/texture/sampler/address_mode.hpp"
#include "base/math/vector.inl"
#include "base/spectrum/rgb.inl"

namespace image { namespace procedural {

Renderer::Renderer(int2 dimensions, int num_sqrt_samples) :
	num_sqrt_samples_(num_sqrt_samples),
	dimensions_(num_sqrt_samples * dimensions),
	dimensions_f_(num_sqrt_samples * dimensions),
	samples_(new float4[num_sqrt_samples * dimensions.x * num_sqrt_samples * dimensions.y]),
	brush_(float4(0.f, 0.f, 0.f, 1.f))
{}

Renderer::~Renderer() {
	delete [] samples_;
}

void Renderer::set_brush(float3_p color) {
	brush_ = float4(color, 1.f);
}

void Renderer::set_brush(const float4& color) {
	brush_ = color;
}

void Renderer::clear() {
	for (int i = 0, len = dimensions_.x * dimensions_.y; i < len; ++i) {
		samples_[i] = brush_;
	}
}

void Renderer::draw_circle(float2 pos, float radius) {
	int2 sample(pos * dimensions_f_);

	int x = static_cast<int>(radius * dimensions_f_.x);
	int y = 0;
	int err = 0;

	while (x >= y) {
		set_row(sample.x - x, sample.x + x, sample.y + y, brush_);
		set_row(sample.x - y, sample.x + y, sample.y + x, brush_);
		set_row(sample.x - x, sample.x + x, sample.y - y, brush_);
		set_row(sample.x - y, sample.x + y, sample.y - x, brush_);

	//	set_sample(sample.x + x, sample.y + y, brush_);
	//	set_sample(sample.x + y, sample.y + x, brush_);
	//	set_sample(sample.x - y, sample.y + x, brush_);
	//	set_sample(sample.x - x, sample.y + y, brush_);
	//	set_sample(sample.x - x, sample.y - y, brush_);
	//	set_sample(sample.x - y, sample.y - x, brush_);
	//	set_sample(sample.x + y, sample.y - x, brush_);
	//	set_sample(sample.x + x, sample.y - y, brush_);

		y += 1;
		err += 1 + 2 * y;

		if (2 * (err - x) + 1 > 0) {
			x -= 1;
			err += 1 - 2 * x;
		}
	}
}

void Renderer::resolve(Image_byte_3& image) const {
	if (1 == num_sqrt_samples_) {
		for (int i = 0, len = image.area(); i < len; ++i) {
			auto& s = samples_[i];

			image.at(i) = spectrum::float_to_snorm(s.xyz);
		}
	} else {
		int num_samples = num_sqrt_samples_ * num_sqrt_samples_;

		float n = 1.f / static_cast<float>(num_samples);

		auto i_d = image.description().dimensions;

		for (int i_y = 0; i_y < i_d.y; ++i_y) {
			int b_y = num_sqrt_samples_ * i_y;
			for (int i_x = 0; i_x < i_d.x; ++i_x) {
				int b_x = num_sqrt_samples_ * i_x;

				float3 result(0.f, 0.f, 0.f);

				for (int y = 0; y < num_sqrt_samples_; ++y) {
					for (int x = 0; x < num_sqrt_samples_; ++x) {
						int s = dimensions_.x * (b_y + y) + b_x + x;

						result += samples_[s].xyz;
					}
				}

				image.at(i_x, i_y) = spectrum::float_to_snorm(n * result);
			}
		}
	}
}

void Renderer::set_sample(int x, int y, const float4& color) {
	x = math::mod(x, dimensions_.x);
	y = math::mod(y, dimensions_.y);

	samples_[dimensions_.x * y + x] = color;
}

void Renderer::set_row(int start_x, int end_x, int y, const float4& color) {
	for (int x = start_x; x < end_x; ++x) {
		set_sample(x, y, color);
	}
}

}}
