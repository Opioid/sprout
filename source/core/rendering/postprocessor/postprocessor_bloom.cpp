#include "postprocessor_bloom.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include "base/spectrum/rgb.inl"

namespace rendering { namespace postprocessor {

class Gaussian_functor {

public:

	Gaussian_functor(float radius_square, float alpha) :
		exp_(std::exp(-alpha * radius_square)),
		alpha_(alpha) {}

	float operator()(float x) {
		return std::max(0.f, std::exp(-alpha_ * x) - exp_);
	}

private:

	float exp_;
	float alpha_;
};

Bloom::Bloom(float angle, float alpha, float threshold, float intensity) :
	Postprocessor(2),
	angle_(angle), alpha_(alpha), threshold_(threshold), intensity_(intensity) {}

void Bloom::init(const scene::camera::Camera& camera) {
	image::Image::Description description(image::Image::Type::Float_4, camera.sensor_dimensions());
	scratch_.resize(description);

	float blur_angle = angle_ * math::Pi;

	float solid_angle = camera.pixel_solid_angle();

	int32_t radius = static_cast<int32_t>(blur_angle / solid_angle + 0.5f);

	int32_t width = 2 * radius + 1;

	kernel_.resize(width);

	float fr = static_cast<float>(radius) + 0.5f;
	Gaussian_functor gauss(static_cast<float>(fr * fr), alpha_);

	for (int32_t x = 0; x < width; ++x) {
		int32_t o = -radius + x;

		float fo = static_cast<float>(o);
		float w = gauss(fo * fo);

		kernel_[x] = K{o, w};
	}
}

void Bloom::apply(int32_t begin, int32_t end, uint32_t pass,
				  const image::Image_float_4& source,
				  image::Image_float_4& destination) {
	float threshold = threshold_;
	float intensity = intensity_;

	if (0 == pass) {
		for (int32_t i = begin; i < end; ++i) {
			int2 c = source.coordinates_2(i);

			float3 accum(0.f);
			float  weight_sum = 0.f;
			for (auto& k : kernel_) {
				int32_t ci = source.checked_index(c + int2(0, k.o));

				if (ci > 0) {
					float3 color = source.at(ci).xyz;

					float l = spectrum::luminance(color);

					if (l > threshold) {
						accum += k.w * color;
					}

					weight_sum += k.w;
				}
			}

			if (weight_sum > 0.f) {
				float3 bloom = accum / weight_sum;
				scratch_.at(i) = float4(bloom);
			} else {
				scratch_.at(i) = float4(0.f);
			}
		}
	} else {
		for (int32_t i = begin; i < end; ++i) {
			int2 c = source.coordinates_2(i);

			float3 accum(0.f);
			float  weight_sum = 0.f;
			for (auto& k : kernel_) {
				int32_t ci = source.checked_index(c + int2(k.o, 0));

				if (ci > 0) {
					float3 bloom = scratch_.at(ci).xyz;

					accum += k.w * bloom;

					weight_sum += k.w;
				}
			}

			const float4& s = source.at(i);

			if (weight_sum > 0.f) {
				float3 bloom = accum / weight_sum;
				destination.at(i) = float4(s.xyz + intensity * bloom, s.w);
			} else {
				destination.at(i) = s;
			}
		}
	}
}

}}
