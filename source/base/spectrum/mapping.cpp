#include "mapping.hpp"
#include "xyz.hpp"

#include <iterator>

// http://www.scratchapixel.com/old/lessons/3d-advanced-lessons/blackbody/spectrum-of-blackbodies/

namespace spectrum {

float3 heatmap(float x) {
    // The last color is intentionally present twice, as a convenient way to catch (1.f == x) cases.
    static float3 constexpr colors[] = {{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f},
                                        {1.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}};

    static int32_t constexpr max_color = int32_t(std::size(colors) - 2);

    int32_t const id = int32_t(x * float(max_color));

    float const ip = x * float(max_color) - float(id);

    return lerp(colors[id], colors[id + 1], ip);
}

byte3 turbo(float x) {
    static unsigned char constexpr turbo_srgb_bytes[256][3] = {
        {48, 18, 59},    {50, 21, 67},    {51, 24, 74},   {52, 27, 81},   {53, 30, 88},
        {54, 33, 95},    {55, 36, 102},   {56, 39, 109},  {57, 42, 115},  {58, 45, 121},
        {59, 47, 128},   {60, 50, 134},   {61, 53, 139},  {62, 56, 145},  {63, 59, 151},
        {63, 62, 156},   {64, 64, 162},   {65, 67, 167},  {65, 70, 172},  {66, 73, 177},
        {66, 75, 181},   {67, 78, 186},   {68, 81, 191},  {68, 84, 195},  {68, 86, 199},
        {69, 89, 203},   {69, 92, 207},   {69, 94, 211},  {70, 97, 214},  {70, 100, 218},
        {70, 102, 221},  {70, 105, 224},  {70, 107, 227}, {71, 110, 230}, {71, 113, 233},
        {71, 115, 235},  {71, 118, 238},  {71, 120, 240}, {71, 123, 242}, {70, 125, 244},
        {70, 128, 246},  {70, 130, 248},  {70, 133, 250}, {70, 135, 251}, {69, 138, 252},
        {69, 140, 253},  {68, 143, 254},  {67, 145, 254}, {66, 148, 255}, {65, 150, 255},
        {64, 153, 255},  {62, 155, 254},  {61, 158, 254}, {59, 160, 253}, {58, 163, 252},
        {56, 165, 251},  {55, 168, 250},  {53, 171, 248}, {51, 173, 247}, {49, 175, 245},
        {47, 178, 244},  {46, 180, 242},  {44, 183, 240}, {42, 185, 238}, {40, 188, 235},
        {39, 190, 233},  {37, 192, 231},  {35, 195, 228}, {34, 197, 226}, {32, 199, 223},
        {31, 201, 221},  {30, 203, 218},  {28, 205, 216}, {27, 208, 213}, {26, 210, 210},
        {26, 212, 208},  {25, 213, 205},  {24, 215, 202}, {24, 217, 200}, {24, 219, 197},
        {24, 221, 194},  {24, 222, 192},  {24, 224, 189}, {25, 226, 187}, {25, 227, 185},
        {26, 228, 182},  {28, 230, 180},  {29, 231, 178}, {31, 233, 175}, {32, 234, 172},
        {34, 235, 170},  {37, 236, 167},  {39, 238, 164}, {42, 239, 161}, {44, 240, 158},
        {47, 241, 155},  {50, 242, 152},  {53, 243, 148}, {56, 244, 145}, {60, 245, 142},
        {63, 246, 138},  {67, 247, 135},  {70, 248, 132}, {74, 248, 128}, {78, 249, 125},
        {82, 250, 122},  {85, 250, 118},  {89, 251, 115}, {93, 252, 111}, {97, 252, 108},
        {101, 253, 105}, {105, 253, 102}, {109, 254, 98}, {113, 254, 95}, {117, 254, 92},
        {121, 254, 89},  {125, 255, 86},  {128, 255, 83}, {132, 255, 81}, {136, 255, 78},
        {139, 255, 75},  {143, 255, 73},  {146, 255, 71}, {150, 254, 68}, {153, 254, 66},
        {156, 254, 64},  {159, 253, 63},  {161, 253, 61}, {164, 252, 60}, {167, 252, 58},
        {169, 251, 57},  {172, 251, 56},  {175, 250, 55}, {177, 249, 54}, {180, 248, 54},
        {183, 247, 53},  {185, 246, 53},  {188, 245, 52}, {190, 244, 52}, {193, 243, 52},
        {195, 241, 52},  {198, 240, 52},  {200, 239, 52}, {203, 237, 52}, {205, 236, 52},
        {208, 234, 52},  {210, 233, 53},  {212, 231, 53}, {215, 229, 53}, {217, 228, 54},
        {219, 226, 54},  {221, 224, 55},  {223, 223, 55}, {225, 221, 55}, {227, 219, 56},
        {229, 217, 56},  {231, 215, 57},  {233, 213, 57}, {235, 211, 57}, {236, 209, 58},
        {238, 207, 58},  {239, 205, 58},  {241, 203, 58}, {242, 201, 58}, {244, 199, 58},
        {245, 197, 58},  {246, 195, 58},  {247, 193, 58}, {248, 190, 57}, {249, 188, 57},
        {250, 186, 57},  {251, 184, 56},  {251, 182, 55}, {252, 179, 54}, {252, 177, 54},
        {253, 174, 53},  {253, 172, 52},  {254, 169, 51}, {254, 167, 50}, {254, 164, 49},
        {254, 161, 48},  {254, 158, 47},  {254, 155, 45}, {254, 153, 44}, {254, 150, 43},
        {254, 147, 42},  {254, 144, 41},  {253, 141, 39}, {253, 138, 38}, {252, 135, 37},
        {252, 132, 35},  {251, 129, 34},  {251, 126, 33}, {250, 123, 31}, {249, 120, 30},
        {249, 117, 29},  {248, 114, 28},  {247, 111, 26}, {246, 108, 25}, {245, 105, 24},
        {244, 102, 23},  {243, 99, 21},   {242, 96, 20},  {241, 93, 19},  {240, 91, 18},
        {239, 88, 17},   {237, 85, 16},   {236, 83, 15},  {235, 80, 14},  {234, 78, 13},
        {232, 75, 12},   {231, 73, 12},   {229, 71, 11},  {228, 69, 10},  {226, 67, 10},
        {225, 65, 9},    {223, 63, 8},    {221, 61, 8},   {220, 59, 7},   {218, 57, 7},
        {216, 55, 6},    {214, 53, 6},    {212, 51, 5},   {210, 49, 5},   {208, 47, 5},
        {206, 45, 4},    {204, 43, 4},    {202, 42, 4},   {200, 40, 3},   {197, 38, 3},
        {195, 37, 3},    {193, 35, 2},    {190, 33, 2},   {188, 32, 2},   {185, 30, 2},
        {183, 29, 2},    {180, 27, 1},    {178, 26, 1},   {175, 24, 1},   {172, 23, 1},
        {169, 22, 1},    {167, 20, 1},    {164, 19, 1},   {161, 18, 1},   {158, 16, 1},
        {155, 15, 1},    {152, 14, 1},    {149, 13, 1},   {146, 11, 1},   {142, 10, 1},
        {139, 9, 2},     {136, 8, 2},     {133, 7, 2},    {129, 6, 2},    {126, 5, 2},
        {122, 4, 3}};

    uint32_t const i = uint32_t(x * 255.f);

    return byte3(turbo_srgb_bytes[i][0], turbo_srgb_bytes[i][1], turbo_srgb_bytes[i][2]);
}

static float planck(float temperature, float wavelength) {
    static float constexpr h = 6.62606896e-34f;  // Plank constant
    static float constexpr c = 2.99792458e+8f;   // Speed of light
    static float constexpr k = 1.38064880e-23f;  // Boltzmann constant
    static float constexpr a = ((2.f * Pi) * h) * (c * c);
    static float constexpr b = (h * c) / k;
    return (a * std::pow(wavelength, -5.f)) / (std::exp(b / (wavelength * temperature)) - 1.f);
}

// CIE color matching functions
static float constexpr color_matching[][3] = {
    {0.0014f, 0.0000f, 0.0065f}, {0.0022f, 0.0001f, 0.0105f}, {0.0042f, 0.0001f, 0.0201f},
    {0.0076f, 0.0002f, 0.0362f}, {0.0143f, 0.0004f, 0.0679f}, {0.0232f, 0.0006f, 0.1102f},
    {0.0435f, 0.0012f, 0.2074f}, {0.0776f, 0.0022f, 0.3713f}, {0.1344f, 0.0040f, 0.6456f},
    {0.2148f, 0.0073f, 1.0391f}, {0.2839f, 0.0116f, 1.3856f}, {0.3285f, 0.0168f, 1.6230f},
    {0.3483f, 0.0230f, 1.7471f}, {0.3481f, 0.0298f, 1.7826f}, {0.3362f, 0.0380f, 1.7721f},
    {0.3187f, 0.0480f, 1.7441f}, {0.2908f, 0.0600f, 1.6692f}, {0.2511f, 0.0739f, 1.5281f},
    {0.1954f, 0.0910f, 1.2876f}, {0.1421f, 0.1126f, 1.0419f}, {0.0956f, 0.1390f, 0.8130f},
    {0.0580f, 0.1693f, 0.6162f}, {0.0320f, 0.2080f, 0.4652f}, {0.0147f, 0.2586f, 0.3533f},
    {0.0049f, 0.3230f, 0.2720f}, {0.0024f, 0.4073f, 0.2123f}, {0.0093f, 0.5030f, 0.1582f},
    {0.0291f, 0.6082f, 0.1117f}, {0.0633f, 0.7100f, 0.0782f}, {0.1096f, 0.7932f, 0.0573f},
    {0.1655f, 0.8620f, 0.0422f}, {0.2257f, 0.9149f, 0.0298f}, {0.2904f, 0.9540f, 0.0203f},
    {0.3597f, 0.9803f, 0.0134f}, {0.4334f, 0.9950f, 0.0087f}, {0.5121f, 1.0000f, 0.0057f},
    {0.5945f, 0.9950f, 0.0039f}, {0.6784f, 0.9786f, 0.0027f}, {0.7621f, 0.9520f, 0.0021f},
    {0.8425f, 0.9154f, 0.0018f}, {0.9163f, 0.8700f, 0.0017f}, {0.9786f, 0.8163f, 0.0014f},
    {1.0263f, 0.7570f, 0.0011f}, {1.0567f, 0.6949f, 0.0010f}, {1.0622f, 0.6310f, 0.0008f},
    {1.0456f, 0.5668f, 0.0006f}, {1.0026f, 0.5030f, 0.0003f}, {0.9384f, 0.4412f, 0.0002f},
    {0.8544f, 0.3810f, 0.0002f}, {0.7514f, 0.3210f, 0.0001f}, {0.6424f, 0.2650f, 0.0000f},
    {0.5419f, 0.2170f, 0.0000f}, {0.4479f, 0.1750f, 0.0000f}, {0.3608f, 0.1382f, 0.0000f},
    {0.2835f, 0.1070f, 0.0000f}, {0.2187f, 0.0816f, 0.0000f}, {0.1649f, 0.0610f, 0.0000f},
    {0.1212f, 0.0446f, 0.0000f}, {0.0874f, 0.0320f, 0.0000f}, {0.0636f, 0.0232f, 0.0000f},
    {0.0468f, 0.0170f, 0.0000f}, {0.0329f, 0.0119f, 0.0000f}, {0.0227f, 0.0082f, 0.0000f},
    {0.0158f, 0.0057f, 0.0000f}, {0.0114f, 0.0041f, 0.0000f}, {0.0081f, 0.0029f, 0.0000f},
    {0.0058f, 0.0021f, 0.0000f}, {0.0041f, 0.0015f, 0.0000f}, {0.0029f, 0.0010f, 0.0000f},
    {0.0020f, 0.0007f, 0.0000f}, {0.0014f, 0.0005f, 0.0000f}, {0.0010f, 0.0004f, 0.0000f},
    {0.0007f, 0.0002f, 0.0000f}, {0.0005f, 0.0002f, 0.0000f}, {0.0003f, 0.0001f, 0.0000f},
    {0.0002f, 0.0001f, 0.0000f}, {0.0002f, 0.0001f, 0.0000f}, {0.0001f, 0.0000f, 0.0000f},
    {0.0001f, 0.0000f, 0.0000f}, {0.0001f, 0.0000f, 0.0000f}, {0.0000f, 0.0000f, 0.0000f}};

float3 blackbody(float temperature) {
    static float constexpr wl_min  = 380.f;
    static float constexpr wl_max  = 780.f;
    static float constexpr wl_step = 5.f;

    static uint32_t constexpr num_steps = uint32_t((wl_max - wl_min) / wl_step) + 1;

    float3 xyz(0.f);
    for (uint32_t k = 0; k < num_steps; ++k) {
        // convert to nanometer
        float const wl = (wl_min + float(k) * wl_step) * 1e-9f;
        float const p  = planck(temperature, wl);

        xyz[0] += p * color_matching[k][0];
        xyz[1] += p * color_matching[k][1];
        xyz[2] += p * color_matching[k][2];
    }

    // normalize the result
    xyz /= std::max(xyz[0], std::max(xyz[1], xyz[2]));

    return math::max(spectrum::XYZ_to_sRGB(xyz), 0.f);
}

}  // namespace spectrum
