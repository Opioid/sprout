#include "property.hpp"

namespace rendering::sensor::aov {

image::Layout layout(Property property) {
    using Encoding = image::Encoding;

    switch (property) {
        case Property::Unknown:
        case Property::Albedo:
            return {Encoding::Color, 3};
        case Property::Geometric_normal:
        case Property::Shading_normal:
            return {Encoding::SNorm, 3};
        case Property::Roughness:
            return {Encoding::UNorm, 1};
        case Property::Material_id:
            return {Encoding::UInt, 1};
        case Property::Depth:
            return {Encoding::Depth, 1};
        case Property::AO:
            return {Encoding::UNorm, 1};
    }
}

Operation operation(Property property) {
    switch (property) {
        case Property::Material_id:
            return Operation::Overwrite;
        case Property::Depth:
            return Operation::Less;
        default:
            return Operation::Accumulate;
    }
}

}  // namespace rendering::sensor::aov
