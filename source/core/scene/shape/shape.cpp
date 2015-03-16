#include "shape.hpp"

namespace scene { namespace shape {

bool Shape::is_complex() const {
	return false;
}

bool Shape::is_finite() const {
	return true;
}

}}
