#ifndef SU_CORE_RENDERING_SENSOR_AOV_PROPERTY_HPP
#define SU_CORE_RENDERING_SENSOR_AOV_PROPERTY_HPP

namespace rendering::sensor::aov {

enum class Property {
    Albedo = 0,
    Roughness,
    Geometric_normal,
    Shading_normal,
    Material_id,
    Unknown
};

bool is_data(Property property);

}  // namespace rendering::sensor::aov

#endif
