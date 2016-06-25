#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace coating {

struct Clearcoat {
	float f0;
	float a2;
	float weight;
};

struct Thinfilm {
	float ior;
	float a2;
	float thickness;
	float weight;
};

template<typename Coating>
struct Coating_layer : Sample::Layer, Coating {
	void set(const Coating& coating);
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}}}
