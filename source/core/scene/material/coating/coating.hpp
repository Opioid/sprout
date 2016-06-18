#pragma once

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

}}}
