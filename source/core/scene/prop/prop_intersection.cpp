#include "prop_intersection.hpp"
#include "prop.hpp"

namespace scene {

rendering::material::Material* Intersection::material() const {
	return prop->material(geo.material_index);
}

}
