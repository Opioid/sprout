#include "interpolated.hpp"
#include "math/math.hpp"

namespace spectrum {

Interpolated::Interpolated(uint32_t len, float const* wavelengths, float const* intensities)
    : num_elements_(len), wavelengths_(new float[len]), intensities_(new float[len]) {
    std::copy(wavelengths, wavelengths + len, wavelengths_);
    std::copy(intensities, intensities + len, intensities_);
}

Interpolated::~Interpolated() {
    delete[] intensities_;
    delete[] wavelengths_;
}

float Interpolated::start_wavelength() const {
    return wavelengths_[0];
}

float Interpolated::end_wavelength() const {
    return wavelengths_[num_elements_ - 1];
}

float Interpolated::evaluate(float wl) const {
    auto const result = std::equal_range(wavelengths_, wavelengths_ + num_elements_, wl);

    uint32_t const index = uint32_t(result.first - wavelengths_);

    if (result.first == result.second) {
        float const wl0 = wavelengths_[index - 1];
        float const wl1 = wavelengths_[index];

        float const i0 = intensities_[index - 1];
        float const i1 = intensities_[index];

        return math::lerp(i0, i1, (wl - wl0) / (wl1 - wl0));
    }

    return intensities_[index];
}

float Interpolated::integrate(float a, float b) const {
    uint32_t const len = num_elements_;
    if (len < 2) {
        return 0.f;
    }

    float const start = std::max(a, start_wavelength());
    float const end   = std::min(b, end_wavelength());
    if (end <= start) {
        return 0.f;
    }

    // This integration is only correct for a linearly interpolated function
    // and clamps to zero outside the given range.

    auto const it = std::lower_bound(wavelengths_, wavelengths_ + len, start);

    uint32_t index = std::max(uint32_t(it - wavelengths_), 1u) - 1;

    float integral = 0.f;
    for (; index + 1 < len && end >= wavelengths_[index]; ++index) {
        float const wl0 = wavelengths_[index];
        float const wl1 = wavelengths_[index + 1];

        float const c0 = std::max(wl0, start);
        float const c1 = std::min(wl1, end);

        if (c1 <= c0) {
            continue;
        }

        float const i0  = intensities_[index];
        float const i1  = intensities_[index + 1];
        float const inv = 1.f / (wl1 - wl0);

        float const interp0 = math::lerp(i0, i1, (c0 - wl0) * inv);
        float const interp1 = math::lerp(i0, i1, (c1 - wl0) * inv);

        integral += 0.5f * (interp0 + interp1) * (c1 - c0);
    }

    return integral;
}

float const RGB_to_spectrum_wavelengths[Num_wavelengths] = {
    380.000000f, 385.396825f, 390.793651f, 396.190476f, 401.587302f, 406.984127f, 412.380952f,
    417.777778f, 423.174603f, 428.571429f, 433.968254f, 439.365079f, 444.761905f, 450.158730f,
    455.555556f, 460.952381f, 466.349206f, 471.746032f, 477.142857f, 482.539683f, 487.936508f,
    493.333333f, 498.730159f, 504.126984f, 509.523810f, 514.920635f, 520.317460f, 525.714286f,
    531.111111f, 536.507937f, 541.904762f, 547.301587f, 552.698413f, 558.095238f, 563.492063f,
    568.888889f, 574.285714f, 579.682540f, 585.079365f, 590.476190f, 595.873016f, 601.269841f,
    606.666667f, 612.063492f, 617.460317f, 622.857143f, 628.253968f, 633.650794f, 639.047619f,
    644.444444f, 649.841270f, 655.238095f, 660.634921f, 666.031746f, 671.428571f, 676.825397f,
    682.222222f, 687.619048f, 693.015873f, 698.412698f, 703.809524f, 709.206349f, 714.603175f,
    720.000000f,
};

// generated with a whitepoint of wx = .3333; wy = .3333;
float const RGB_reflector_to_spectrum_white[Num_wavelengths] = {
    1.061030f, 1.059838f, 1.062381f, 1.062877f, 1.063021f, 1.061923f, 1.061572f, 1.062888f,
    1.061735f, 1.063289f, 1.063161f, 1.062499f, 1.061470f, 1.062636f, 1.062777f, 1.062895f,
    1.061217f, 1.061067f, 1.061336f, 1.062410f, 1.063127f, 1.063037f, 1.063283f, 1.061749f,
    1.062540f, 1.061177f, 1.062098f, 1.060538f, 1.059377f, 1.061063f, 1.062865f, 1.060886f,
    1.062381f, 1.062384f, 1.063466f, 1.061229f, 1.063220f, 1.063023f, 1.062498f, 1.063319f,
    1.062335f, 1.062652f, 1.061961f, 1.063016f, 1.061222f, 1.063538f, 1.062553f, 1.061745f,
    1.061511f, 1.062751f, 1.061349f, 1.061164f, 1.061145f, 1.063114f, 1.062570f, 1.059913f,
    1.059869f, 1.062575f, 1.059916f, 1.061512f, 1.062366f, 1.062366f, 1.062525f, 1.062799f,
};

float const RGB_reflector_to_spectrum_cyan[Num_wavelengths] = {
    1.051672f, 1.054489f, 1.053767f, 1.039506f, 1.025542f, 1.012466f, 1.001296f, 0.993209f,
    0.990312f, 0.995077f, 1.008785f, 1.029902f, 1.054126f, 1.051265f, 1.054520f, 1.054375f,
    1.054375f, 1.054112f, 1.054381f, 1.053659f, 1.054385f, 1.054433f, 1.054494f, 1.054539f,
    1.054434f, 1.054237f, 1.054439f, 1.054493f, 1.054491f, 1.054430f, 1.054277f, 1.054424f,
    1.054539f, 1.054501f, 1.054521f, 1.054169f, 1.054169f, 0.928620f, 0.732266f, 0.507465f,
    0.292186f, 0.117396f, 0.011849f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000101f, 0.000100f, 0.000127f, 0.000181f, 0.000165f, 0.002356f, 0.003305f, 0.003581f,
};

float const RGB_reflector_to_spectrum_magenta[Num_wavelengths] = {
    1.005960f, 1.005414f,  1.003419f,  1.006313f,  1.006970f,  1.006177f,  1.006667f,  1.006639f,
    1.006342f, 1.006533f,  1.006547f,  1.006796f,  1.006420f,  1.006834f,  1.004855f,  1.005941f,
    1.006866f, 1.006968f,  1.006970f,  0.955928f,  0.775710f,  0.531572f,  0.286389f,  0.089253f,
    0.005278f, -0.000028f, -0.002180f, -0.002871f, -0.002494f, -0.001886f, -0.001075f, -0.000071f,
    0.001112f, 0.002683f,  0.006203f,  0.011105f,  0.082030f,  0.251454f,  0.478924f,  0.703292f,
    0.895387f, 1.002884f,  1.006505f,  1.006575f,  1.006773f,  1.006648f,  1.005659f,  1.006503f,
    1.006474f, 1.005693f,  1.006694f,  1.006565f,  1.006628f,  1.006604f,  1.006696f,  1.005397f,
    1.006668f, 1.004931f,  1.004739f,  1.006750f,  1.005938f,  1.006553f,  1.006328f,  1.005428f,
};

float const RGB_reflector_to_spectrum_yellow[Num_wavelengths] = {
    0.049450f, 0.047064f, 0.046437f, 0.044339f, 0.043039f, 0.041840f, 0.040256f, 0.040950f,
    0.036703f, 0.041124f, 0.038681f, 0.035959f, 0.039774f, 0.049554f, 0.065018f, 0.092801f,
    0.127998f, 0.179587f, 0.251809f, 0.336173f, 0.438699f, 0.548859f, 0.666307f, 0.797719f,
    0.926099f, 1.049559f, 1.049172f, 1.049867f, 1.046958f, 1.046609f, 1.049866f, 1.049826f,
    1.049863f, 1.049841f, 1.049846f, 1.049946f, 1.049558f, 1.049466f, 1.049922f, 1.049777f,
    1.049779f, 1.049684f, 1.049947f, 1.049897f, 1.049867f, 1.049855f, 1.049239f, 1.049232f,
    1.049801f, 1.049841f, 1.049394f, 1.049807f, 1.049818f, 1.049777f, 1.049933f, 1.049822f,
    1.049756f, 1.049651f, 1.049801f, 1.046984f, 1.049946f, 1.049456f, 1.049784f, 1.049854f,
};

float const RGB_reflector_to_spectrum_red[Num_wavelengths] = {
    0.170703f, 0.168403f, 0.167387f, 0.165440f, 0.163462f, 0.155088f, 0.142358f, 0.131586f,
    0.106832f, 0.073255f, 0.044089f, 0.014094f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.032369f, 0.351654f, 0.744498f,
    1.004733f, 1.004527f, 1.004756f, 1.004755f, 1.004738f, 1.004688f, 1.004751f, 1.004753f,
    1.004749f, 1.004729f, 1.004702f, 1.003914f, 1.004744f, 1.004360f, 1.004734f, 1.003129f,
    1.004487f, 1.004335f, 1.004087f, 1.004740f, 1.003911f, 1.004729f, 1.004195f, 1.004756f,
};

float const RGB_reflector_to_spectrum_green[Num_wavelengths] = {
    0.000753f, 0.000300f, 0.001266f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.004897f, 0.106573f, 0.261636f, 0.457782f, 0.665053f, 0.848654f, 0.992186f, 0.999885f,
    0.999952f, 0.999922f, 0.999955f, 0.999974f, 0.999954f, 0.999962f, 0.999916f, 0.999966f,
    0.999958f, 0.999985f, 0.999529f, 0.999894f, 0.974529f, 0.868339f, 0.712902f, 0.529373f,
    0.336848f, 0.174599f, 0.053530f, 0.001638f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.006536f, 0.000100f, 0.000100f, 0.000100f, 0.004099f, 0.007370f, 0.008285f, 0.004014f,
};

float const RGB_reflector_to_spectrum_blue[Num_wavelengths] = {
    0.947148f, 0.948021f, 0.945754f, 0.967222f, 0.985207f, 0.975869f, 0.999998f, 1.000000f,
    0.999498f, 0.996392f, 0.999524f, 0.999971f, 0.999970f, 0.999986f, 0.999890f, 1.000000f,
    0.984324f, 0.945557f, 0.844596f, 0.733946f, 0.616532f, 0.498881f, 0.386794f, 0.278527f,
    0.182345f, 0.102627f, 0.042541f, 0.006867f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f, 0.000100f,
    0.000100f, 0.000100f, 0.004070f, 0.010824f, 0.020398f, 0.029300f, 0.035446f, 0.041237f,
    0.045790f, 0.049579f, 0.052427f, 0.053220f, 0.055300f, 0.057031f, 0.058071f, 0.058072f,
    0.058274f, 0.058004f, 0.058075f, 0.058514f, 0.058566f, 0.058930f, 0.059622f, 0.059874f,
};

}  // namespace spectrum
