#ifndef SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP
#define SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP

namespace thread { class Pool; }

namespace scene::shape {

class Morphable_shape {

public:

	virtual void morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) = 0;
};

}

#endif
