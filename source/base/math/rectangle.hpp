#pragma once

#include "vector2.hpp"
#include <cstdint>

namespace math {

template<typename T>
struct Rectangle {
	Vector2<T> start, end;
};

typedef Rectangle<int32_t> Recti;

}
