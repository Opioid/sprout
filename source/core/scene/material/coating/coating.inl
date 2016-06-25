#pragma once

#include "coating.hpp"

namespace scene { namespace material { namespace coating {

inline void Clearcoat::set(float f0, float a2, float weight) {
	this->f0 = f0;
	this->a2 = a2;
	this->weight = weight;
}

inline void Thinfilm::set(float ior, float a2, float thickness, float weight) {
	this->ior = ior;
	this->a2  = a2;
	this->thickness = thickness;
	this->weight = weight;
}

}}}
