#include "postprocessor_glare.hpp"
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

void Glare::init(const scene::camera::Camera& /*camera*/) {
	int32_t radius = 16;

	int32_t width = 2 * radius + 1;

//	kernel_.resize(width * width);

	Gaussian_functor gauss(static_cast<float>(radius * radius), 0.0025f);

	for (int32_t y = 0; y < width; ++y) {
		for (int32_t x = 0; x < width; ++x) {
		//	int32_t i = y * width + x;

			int2 p(-radius + x, -radius + y);

			float2 fp(p);
			float w = gauss(fp.x * fp.x) * gauss(fp.y * fp.y);

			if (w > 0.f) {
				kernel_.push_back(K{p, w});
			}
		}
	}
}

void Glare::apply(int32_t begin, int32_t end,
				  const image::Image_float_4& source,
				  image::Image_float_4& destination) const {
	for (int32_t i = begin; i < end; ++i) {
		int2 c = source.coordinates_2(i);

		float3 accum(0.f);
		float  weight_sum = 0.f;
		for (auto& k : kernel_) {
			int32_t ci = source.checked_index(c + k.p);

			if (ci > 0) {
				float3 color = source.at(ci).xyz;

				float l = spectrum::luminance(color);

				if (l > 0.75f) {
					accum += k.w * color;
				}

				weight_sum += k.w;
			}
		}

		const float4& s = source.at(i);

		if (weight_sum > 0.f) {
			destination.at(i) = float4(s.xyz + (accum / weight_sum), s.w);
		} else {
			destination.at(i) = s;
		}

//		if (weight_sum > 0.f) {
//			destination.at(i) = float4(accum / weight_sum, 1.f);
//		}
	}
}

}}
