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
    }
}

}  // namespace rendering::sensor::aov
