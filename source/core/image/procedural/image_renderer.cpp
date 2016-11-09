#include "image_renderer.hpp"
#include "image/typed_image.inl"
#include "image/texture/sampler/address_mode.hpp"
#include "base/math/vector.inl"
#include "base/spectrum/rgb.inl"

namespace image { namespace procedural {

Renderer::Renderer(int2 dimensions, int32_t sqrt_num_samples) :
	sqrt_num_samples_(sqrt_num_samples),
	dimensions_(sqrt_num_samples * dimensions),
	dimensions_f_(sqrt_num_samples * dimensions),
	samples_(new float4[sqrt_num_samples * dimensions.x * sqrt_num_samples * dimensions.y]),
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
	for (int32_t i = 0, len = dimensions_.x * dimensions_.y; i < len; ++i) {
		samples_[i] = brush_;
	}
}

void Renderer::draw_circle(float2 pos, float radius) {
	int2 sample(pos * dimensions_f_);

	int32_t x = static_cast<int>(radius * dimensions_f_.x);
	int32_t y = 0;
	int32_t err = 0;

	while (x >= y) {
		set_row(sample.x - x, sample.x + x, sample.y + y, brush_);
		set_row(sample.x - y, sample.x + y, sample.y + x, brush_);
		set_row(sample.x - x, sample.x + x, sample.y - y, brush_);
		set_row(sample.x - y, sample.x + y, sample.y - x, brush_);

		y += 1;
		err += 1 + 2 * y;

		if (2 * (err - x) + 1 > 0) {
			x -= 1;
			err += 1 - 2 * x;
		}
	}
}

void Renderer::resolve_sRGB(Image_byte_3& image) const {
	if (1 == sqrt_num_samples_) {
		for (int32_t i = 0, len = image.area(); i < len; ++i) {
			auto s = samples_[i];

			byte3 srgb = spectrum::float_to_unorm(spectrum::linear_RGB_to_sRGB(s.xyz));
			image.store(i, srgb);
		}
	} else {
		int32_t num_samples = sqrt_num_samples_ * sqrt_num_samples_;

		float n = 1.f / static_cast<float>(num_samples);

		auto i_d = image.description().dimensions;

		for (int32_t i_y = 0; i_y < i_d.y; ++i_y) {
			int32_t b_y = sqrt_num_samples_ * i_y;
			for (int32_t i_x = 0; i_x < i_d.x; ++i_x) {
				int32_t b_x = sqrt_num_samples_ * i_x;

				float3 result(0.f);

				for (int32_t y = 0; y < sqrt_num_samples_; ++y) {
					int32_t b_o = dimensions_.x * (b_y + y) + b_x;
					for (int32_t x = 0; x < sqrt_num_samples_; ++x) {
						int32_t s = b_o + x;
						result += samples_[s].xyz;
					}
				}

				byte3 srgb = spectrum::float_to_unorm(spectrum::linear_RGB_to_sRGB(n * result));
				image.store(i_x, i_y, srgb);
			}
		}
	}
}

void Renderer::resolve(Image_byte_3& image) const {
	if (1 == sqrt_num_samples_) {
		for (int32_t i = 0, len = image.area(); i < len; ++i) {
			auto s = samples_[i];
			image.at(i) = spectrum::float_to_snorm(s.xyz);
		}
	} else {
		int32_t num_samples = sqrt_num_samples_ * sqrt_num_samples_;

		float n = 1.f / static_cast<float>(num_samples);

		auto i_d = image.description().dimensions;

		for (int32_t i_y = 0; i_y < i_d.y; ++i_y) {
			int32_t b_y = sqrt_num_samples_ * i_y;
			for (int32_t i_x = 0; i_x < i_d.x; ++i_x) {
				int32_t b_x = sqrt_num_samples_ * i_x;

				float3 result(0.f);

				for (int32_t y = 0; y < sqrt_num_samples_; ++y) {
					int32_t b_o = dimensions_.x * (b_y + y) + b_x;
					for (int32_t x = 0; x < sqrt_num_samples_; ++x) {
						int32_t s = b_o + x;
						result += samples_[s].xyz;
					}
				}

				image.at(i_x, i_y) = spectrum::float_to_snorm(n * result);
			}
		}
	}
}

void Renderer::resolve(Image_byte_1& image) const {
	if (1 == sqrt_num_samples_) {
		for (int32_t i = 0, len = image.area(); i < len; ++i) {
			auto& s = samples_[i];

			image.at(i) = spectrum::float_to_unorm(s.x);
		}
	} else {
		int32_t num_samples = sqrt_num_samples_ * sqrt_num_samples_;

		float n = 1.f / static_cast<float>(num_samples);

		auto i_d = image.description().dimensions;

		for (int32_t i_y = 0; i_y < i_d.y; ++i_y) {
			int32_t b_y = sqrt_num_samples_ * i_y;
			for (int32_t i_x = 0; i_x < i_d.x; ++i_x) {
				int32_t b_x = sqrt_num_samples_ * i_x;

				float result = 0.f;

				for (int32_t y = 0; y < sqrt_num_samples_; ++y) {
					for (int32_t x = 0; x < sqrt_num_samples_; ++x) {
						int32_t s = dimensions_.x * (b_y + y) + b_x + x;

						result += samples_[s].x;
					}
				}

				image.at(i_x, i_y) = spectrum::float_to_unorm(n * result);
			}
		}
	}
}

void Renderer::set_sample(int32_t x, int32_t y, const float4& color) {
	x = math::mod(x, dimensions_.x);
	y = math::mod(y, dimensions_.y);

	samples_[dimensions_.x * y + x] = color;
}

void Renderer::set_row(int32_t start_x, int32_t end_x, int32_t y, const float4& color) {
	for (int32_t x = start_x; x < end_x; ++x) {
		set_sample(x, y, color);
	}
}

}}
