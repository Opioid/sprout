#pragma once

#include "coating.hpp"

namespace scene { namespace material { namespace coating {

template<typename Coating>
void Coating_layer<Coating>::set(const Coating& coating) {
	static_cast<Coating&>(*this) = coating;
}

}}}
