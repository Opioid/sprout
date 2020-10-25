#include "property.hpp"

namespace rendering::sensor::aov {

bool is_data(Property property) {
    if (Property::Albedo == property) {
        return false;
    }

    return true;
}

}  // namespace rendering::sensor::aov
