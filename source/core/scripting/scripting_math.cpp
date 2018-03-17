#include "scripting_math.hpp"

#include <cmath>

namespace scripting {

double Math::sin(double x) const {
	return std::sin(x);
}

double Math::cos(double x) const {
	return std::cos(x);
}

}
