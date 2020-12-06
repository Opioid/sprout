#include "property.hpp"

namespace rendering::sensor::aov {

image::Encoding encoding(Property property) {
    using Encoding = image::Encoding;

    switch (property) {
        case Property::Unknown:
        case Property::Albedo:
            return Encoding::Color;
        case Property::Geometric_normal:
        case Property::Shading_normal:
            return Encoding::SNorm;
        case Property::Roughness:
            return Encoding::UNorm;
        case Property::Material_id:
            return Encoding::UInt;
        case Property::Depth:
            return Encoding::UNorm;
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
