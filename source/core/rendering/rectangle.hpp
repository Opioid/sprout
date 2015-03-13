#pragma once

#include "base/math/vector2.hpp"
#include <cstdint>

namespace rendering {

template<typename T>
struct Rectangle {
	math::Vector2<T> start, end;
};

typedef Rectangle<uint32_t> Rectui;

}
