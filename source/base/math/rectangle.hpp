#pragma once

#include "vector2.hpp"
#include <cstdint>

namespace math {

template<typename T>
struct Rectangle {
	Vector2<T> start;
	Vector2<T> end;
};

using Recti = Rectangle<int32_t>;

}
