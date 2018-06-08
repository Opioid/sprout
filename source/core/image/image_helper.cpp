#include "image_helper.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "typed_image.inl"

namespace image {

float3 average_and_max_3(Float4 const& image, float3& max) {
  int32_t const len = image.volume();

  float const ilen = 1.f / static_cast<float>(len);

  float3 average(0.f);
  max = float3(0.f);

  for (int32_t i = 0; i < len; ++i) {
    float4 const& pixel = image.at(i);
    float3 pixel3 = pixel.xyz();
    average += ilen * pixel3;
    max = float3(std::max(pixel3[0], max[0]), std::max(pixel3[1], max[1]),
                 std::max(pixel3[2], max[2]));
  }

  return average;
}

float average_and_max_luminance(Float4 const& image, float& max) {
  int32_t const len = image.volume();

  float const ilen = 1.f / static_cast<float>(len);

  float average = 0.f;
  max = 0.f;

  for (int32_t i = 0; i < len; ++i) {
    float luminance = spectrum::luminance(image.at(i).xyz());
    average += ilen * luminance;
    max = std::max(luminance, max);
  }

  return average;
}

}  // namespace image
