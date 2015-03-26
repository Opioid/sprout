#include "prop_intersection.hpp"
#include "prop.hpp"

namespace scene {

material::IMaterial& Intersection::material() const {
	return prop->material(geo.material_index);
}

}
