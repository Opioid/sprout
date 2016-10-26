#include "postprocessor_bloom.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include "base/math/filter/gaussian.hpp"
#include "base/spectrum/rgb.inl"

namespace rendering { namespace postprocessor {

Bloom::Bloom(float angle, float alpha, float threshold, float intensity) :
	Postprocessor(2),
	angle_(angle), alpha_(alpha), threshold_(threshold), intensity_(intensity) {}

void Bloom::init(const scene::camera::Camera& camera) {
	image::Image::Description description(image::Image::Type::Float_4, camera.sensor_dimensions());
	scratch_.resize(description);

	float solid_angle = camera.pixel_solid_angle();

	int32_t radius = static_cast<int32_t>(angle_ / solid_angle + 0.5f);

	int32_t width = 2 * radius + 1;

	kernel_.resize(width);

	float fr = static_cast<float>(radius) + 0.5f;
	math::filter::Gaussian_functor gauss(static_cast<float>(fr * fr), alpha_);

	for (int32_t x = 0; x < width; ++x) {
		int32_t o = -radius + x;

		float fo = static_cast<float>(o);
		float w = gauss(fo * fo);

		kernel_[x] = K{o, w};
	}
}

size_t Bloom::num_bytes() const {
	return sizeof(*this) + scratch_.num_bytes() + kernel_.size() * sizeof(K);
}

void Bloom::apply(int32_t begin, int32_t end, uint32_t pass,
				  const image::Image_float_4& source,
				  image::Image_float_4& destination) {
	float threshold = threshold_;
	float intensity = intensity_;

	auto d = destination.description().dimensions;

	if (0 == pass) {
		for (int32_t i = begin; i < end; ++i) {
			int2 c = source.coordinates_2(i);

			float3 accum(0.f);
			float  weight_sum = 0.f;
			for (auto& k : kernel_) {
				int32_t kx = c.x + k.o;

				if (kx >= 0 && kx < d.x) {
					float3 color = source.load(kx, c.y).xyz;
					float l = spectrum::luminance(color);

					if (l > threshold) {
						accum += k.w * color;
					}

					weight_sum += k.w;
				}
			}

			if (weight_sum > 0.f) {
				float3 bloom = accum / weight_sum;
				scratch_.store(i, float4(bloom));
			} else {
				scratch_.store(i, float4(0.f));
			}
		}
	} else {
		for (int32_t i = begin; i < end; ++i) {
			int2 c = source.coordinates_2(i);

			float3 accum(0.f);
			float  weight_sum = 0.f;
			for (auto& k : kernel_) {
				int32_t ky = c.y + k.o;

				if (ky >= 0 && ky < d.y) {
					float3 bloom = scratch_.load(c.x, ky).xyz;
					accum += k.w * bloom;
					weight_sum += k.w;
				}
			}

			float4 s = source.load(i);

			if (weight_sum > 0.f) {
				float3 bloom = accum / weight_sum;
				destination.store(i, float4(s.xyz + intensity * bloom, s.w));
			} else {
				destination.store(i, s);
			}
		}
	}
}

}}
