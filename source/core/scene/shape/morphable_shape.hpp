#pragma once

namespace thread { class Pool; }

namespace scene { namespace shape {

class Morphable_shape {

public:

	virtual void morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) = 0;
};

}}
