#pragma once

namespace scene { namespace shape { namespace triangle {

struct Coordinates {
	float t, u, v;
};

struct Intersection {
	Coordinates c;
	uint32_t index;
};

}}}

