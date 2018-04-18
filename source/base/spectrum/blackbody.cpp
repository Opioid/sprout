#include "blackbody.hpp"
#include "xyz.hpp"

// http://www.scratchapixel.com/old/lessons/3d-advanced-lessons/blackbody/spectrum-of-blackbodies/

namespace spectrum {

float planck(float temperature, float wavelength) {
	static constexpr float h = 6.62606896e-34f;   // Plank constant
	static constexpr float c = 2.99792458e+8f;    // Speed of light
	static constexpr float k = 1.38064880e-23f;   // Boltzmann constant
	static constexpr float a = ((2.f * math::Pi) * h) * (c * c);
	static constexpr float b = (h * c) / k;
	return (a * std::pow(wavelength, -5.f)) /
		   (std::exp(b / (wavelength * temperature)) - 1.f);
}

// CIE color matching functions
static constexpr float color_matching[][3] = {
	{ 0.0014f, 0.0000f, 0.0065f }, { 0.0022f, 0.0001f, 0.0105f }, { 0.0042f, 0.0001f, 0.0201f },
	{ 0.0076f, 0.0002f, 0.0362f }, { 0.0143f, 0.0004f, 0.0679f }, { 0.0232f, 0.0006f, 0.1102f },
	{ 0.0435f, 0.0012f, 0.2074f }, { 0.0776f, 0.0022f, 0.3713f }, { 0.1344f, 0.0040f, 0.6456f },
	{ 0.2148f, 0.0073f, 1.0391f }, { 0.2839f, 0.0116f, 1.3856f }, { 0.3285f, 0.0168f, 1.6230f },
	{ 0.3483f, 0.0230f, 1.7471f }, { 0.3481f, 0.0298f, 1.7826f }, { 0.3362f, 0.0380f, 1.7721f },
	{ 0.3187f, 0.0480f, 1.7441f }, { 0.2908f, 0.0600f, 1.6692f }, { 0.2511f, 0.0739f, 1.5281f },
	{ 0.1954f, 0.0910f, 1.2876f }, { 0.1421f, 0.1126f, 1.0419f }, { 0.0956f, 0.1390f, 0.8130f },
	{ 0.0580f, 0.1693f, 0.6162f }, { 0.0320f, 0.2080f, 0.4652f }, { 0.0147f, 0.2586f, 0.3533f },
	{ 0.0049f, 0.3230f, 0.2720f }, { 0.0024f, 0.4073f, 0.2123f }, { 0.0093f, 0.5030f, 0.1582f },
	{ 0.0291f, 0.6082f, 0.1117f }, { 0.0633f, 0.7100f, 0.0782f }, { 0.1096f, 0.7932f, 0.0573f },
	{ 0.1655f, 0.8620f, 0.0422f }, { 0.2257f, 0.9149f, 0.0298f }, { 0.2904f, 0.9540f, 0.0203f },
	{ 0.3597f, 0.9803f, 0.0134f }, { 0.4334f, 0.9950f, 0.0087f }, { 0.5121f, 1.0000f, 0.0057f },
	{ 0.5945f, 0.9950f, 0.0039f }, { 0.6784f, 0.9786f, 0.0027f }, { 0.7621f, 0.9520f, 0.0021f },
	{ 0.8425f, 0.9154f, 0.0018f }, { 0.9163f, 0.8700f, 0.0017f }, { 0.9786f, 0.8163f, 0.0014f },
	{ 1.0263f, 0.7570f, 0.0011f }, { 1.0567f, 0.6949f, 0.0010f }, { 1.0622f, 0.6310f, 0.0008f },
	{ 1.0456f, 0.5668f, 0.0006f }, { 1.0026f, 0.5030f, 0.0003f }, { 0.9384f, 0.4412f, 0.0002f },
	{ 0.8544f, 0.3810f, 0.0002f }, { 0.7514f, 0.3210f, 0.0001f }, { 0.6424f, 0.2650f, 0.0000f },
	{ 0.5419f, 0.2170f, 0.0000f }, { 0.4479f, 0.1750f, 0.0000f }, { 0.3608f, 0.1382f, 0.0000f },
	{ 0.2835f, 0.1070f, 0.0000f }, { 0.2187f, 0.0816f, 0.0000f }, { 0.1649f, 0.0610f, 0.0000f },
	{ 0.1212f, 0.0446f, 0.0000f }, { 0.0874f, 0.0320f, 0.0000f }, { 0.0636f, 0.0232f, 0.0000f },
	{ 0.0468f, 0.0170f, 0.0000f }, { 0.0329f, 0.0119f, 0.0000f }, { 0.0227f, 0.0082f, 0.0000f },
	{ 0.0158f, 0.0057f, 0.0000f }, { 0.0114f, 0.0041f, 0.0000f }, { 0.0081f, 0.0029f, 0.0000f },
	{ 0.0058f, 0.0021f, 0.0000f }, { 0.0041f, 0.0015f, 0.0000f }, { 0.0029f, 0.0010f, 0.0000f },
	{ 0.0020f, 0.0007f, 0.0000f }, { 0.0014f, 0.0005f, 0.0000f }, { 0.0010f, 0.0004f, 0.0000f },
	{ 0.0007f, 0.0002f, 0.0000f }, { 0.0005f, 0.0002f, 0.0000f }, { 0.0003f, 0.0001f, 0.0000f },
	{ 0.0002f, 0.0001f, 0.0000f }, { 0.0002f, 0.0001f, 0.0000f }, { 0.0001f, 0.0000f, 0.0000f },
	{ 0.0001f, 0.0000f, 0.0000f }, { 0.0001f, 0.0000f, 0.0000f }, { 0.0000f, 0.0000f, 0.0000f }
};

float3 blackbody(float temperature) {
	static constexpr float wl_min = 380.f;
	static constexpr float wl_max = 780.f;
	static constexpr float wl_step = 5.f;
	static constexpr uint32_t num_steps = static_cast<uint32_t>((wl_max - wl_min) / wl_step) + 1;

	float3 xyz(0.f);
	for (uint32_t k = 0; k < num_steps; ++k) {
		// convert to nanometer
		const float wl = (wl_min + static_cast<float>(k) * wl_step) * 1e-9f;
		const float p = planck(temperature, wl);

		xyz[0] += p * color_matching[k][0];
		xyz[1] += p * color_matching[k][1];
		xyz[2] += p * color_matching[k][2];
	}

	// normalize the result
	xyz /= std::max(xyz[0], std::max(xyz[1], xyz[2]));

//	return spectrum::XYZ_to_linear_RGB(xyz);

	const float3 rgb = math::max(spectrum::XYZ_to_linear_RGB(xyz), float3(0.f));

	return math::normalize(rgb);
}

float3 blackbody_fast(float temperature) {
	float arg1 = 1e9f / (temperature * temperature * temperature);
	float arg2 = 1e6f / (temperature * temperature);
	float arg3 = 1e3f / temperature;

	float xc = 0.f;
	if (temperature >= 1667.f && temperature <= 4000.f) {
		xc = -0.2661239f * arg1 - 0.2343580f * arg2 + 0.8776956f * arg3 + 0.179910f;
	} else if (temperature > 4000.f && temperature <= 25000.f) {
		xc = -3.0258469f * arg1 + 2.1070379f * arg2 + 0.2226347f * arg3 + 0.240390f;
	}

	float xc3 = xc * xc * xc;
	float xc2 = xc * xc;

	float yc = 0.f;
	if (temperature >= 1667.f && temperature <= 2222.f) {
		yc = -1.1063814f * xc3 - 1.34811020f * xc2 + 2.18555832f * xc - 0.20219683f;
	} else if (temperature > 2222.f && temperature <= 4000.f) {
		yc = -0.9549476f * xc3 - 1.37418593f * xc2 + 2.09137015f * xc - 0.16748867f;
	} else if (temperature > 4000.f && temperature <= 25000.f) {
		yc =  3.0817580f * xc3 - 5.87338670f * xc2 + 3.75112997f * xc - 0.37001483f;
	}

	float3 xyz(xc / yc, 1.f, (1.f - xc - yc) / yc);

	xyz /= std::max(xyz[0], std::max(xyz[1], xyz[2]));

	return /*math::normalize*/(spectrum::XYZ_to_linear_RGB(xyz));
}

}
