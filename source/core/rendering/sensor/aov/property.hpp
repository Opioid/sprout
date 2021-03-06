#ifndef SU_CORE_RENDERING_SENSOR_AOV_PROPERTY_HPP
#define SU_CORE_RENDERING_SENSOR_AOV_PROPERTY_HPP

#include "image/channels.hpp"

#include <cstdint>
#include <limits>

namespace rendering::sensor::aov {

enum class Property {
    Albedo = 0,
    Roughness,
    Geometric_normal,
    Shading_normal,
    Material_id,
    Depth,
    AO,
    Unknown
};

struct Descriptor {
    Property property;

    float fparam;
};

enum class Operation : uint8_t { Accumulate = 0, Overwrite, Less };

image::Layout layout(Property property);

Operation operation(Property property);

inline float initial_value(Operation operation) {
    if (Operation::Less == operation) {
        return std::numeric_limits<float>::max();
    }

    return 0.f;
}

}  // namespace rendering::sensor::aov

#endif
