#include "exporting_sink_statistics.hpp"
#include "base/math/vector4.inl"
#include "base/string/string.hpp"
#include "image/image_helper.hpp"
#include "logging/logging.hpp"

namespace exporting {

Statistics::Statistics() = default;

Statistics::~Statistics() = default;

void Statistics::write(image::Float4 const& image, uint32_t frame, Threads& /*threads*/) {
    logging::info("Frame: " + string::to_string(frame));

    float3 max_rgb;
    float3 average_rgb = image::average_and_max_3(image, max_rgb);

    float max_luminance;
    float average_luminance = image::average_and_max_luminance(image, max_luminance);

    logging::info("Average RGB:       " + string::to_string(average_rgb));
    logging::info("Max     RGB:       " + string::to_string(max_rgb));

    logging::info("Average luminance: " + string::to_string(average_luminance) + " (" +
                  string::to_string(683.f * average_luminance) + " cd/m^2)");

    logging::info("Max     luminance: " + string::to_string(max_luminance) + " (" +
                  string::to_string(683.f * max_luminance) + " cd/m^2)");
}

}  // namespace exporting
