#include "aurora_spectrum.hpp"
#include "base/math/function/interpolated_function.inl"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/xyz.hpp"

#include "base/math/print.hpp"
#include <iostream>

namespace procedural::aurora::spectrum {

using namespace ::spectrum;

float3 cie_427;
float3 cie_557;
float3 cie_630;

math::Interpolated_function<float> v_427;
math::Interpolated_function<float> v_557;
math::Interpolated_function<float> v_630;

void init() {


	const Interpolated CIE_X(CIE_Wavelengths_360_830_1nm, CIE_X_360_830_1nm, CIE_XYZ_Num);
	const Interpolated CIE_Y(CIE_Wavelengths_360_830_1nm, CIE_Y_360_830_1nm, CIE_XYZ_Num);
	const Interpolated CIE_Z(CIE_Wavelengths_360_830_1nm, CIE_Z_360_830_1nm, CIE_XYZ_Num);

	constexpr float wl_427 = 427.8f;
	cie_427 = float3(CIE_X.evaluate(wl_427),
					 CIE_Y.evaluate(wl_427),
					 CIE_Z.evaluate(wl_427));

	constexpr float wl_557 = 557.7f;
	cie_557 = float3(CIE_X.evaluate(wl_557),
					 CIE_Y.evaluate(wl_557),
					 CIE_Z.evaluate(wl_557));

	constexpr float wl_630 = 630.f;
	cie_630 = float3(CIE_X.evaluate(wl_630),
					 CIE_Y.evaluate(wl_630),
					 CIE_Z.evaluate(wl_630));

	constexpr float intensities_427[] = {
		0.f,
		0.21f,
		0.39f,
		0.49f,
		0.505f,
		0.47f,
		// 160
		0.4f,
		0.33f,
		0.28f,
		0.21f,
		0.19f,
		0.12f,
		// 220
		0.09f,
		0.05f,
		0.01f,

		0.f
	};

	constexpr float intensities_557[] = {
		0.f,
		0.42f,
		0.79f,
		0.95f,
		1.f,
		0.91f,
		// 160,
		0.815f,
		0.7f,
		0.6f,
		0.48f,
		0.38f,
		0.29f,
		// 220
		0.205f,
		0.12f,
		0.06f,

		0.f
	};

	constexpr float intensities_630[] = {
		0.f,
		0.f,
		0.f,
		0.f,
		0.f,
		0.f,
		// 160
		0.f,
		0.0125f,
		0.05f,
		0.08f,
		0.099f,
		0.1f,
		//220
		0.11f,
		0.12f,
		0.16f,

		0.15f
	};

	constexpr size_t num_elements = sizeof(intensities_427) / sizeof(float);
	v_427.from_array(0.f, 1.f, num_elements, intensities_427);
	v_557.from_array(0.f, 1.f, num_elements, intensities_557);
	v_630.from_array(0.f, 1.f, num_elements, intensities_630);
}

float3 linear_rgb(float normalized_height) {
	float3 xyz(0.f);

	xyz += v_427(normalized_height) * cie_427;
	xyz += v_557(normalized_height) * cie_557;
	xyz += v_630(normalized_height) * cie_630;

	xyz /= 2.1248f;

	float3 rgb = XYZ_to_linear_RGB(xyz);

	return math::saturate(rgb);
}

}
