#ifndef SU_CORE_SCENE_ANIMATION_ANIMATION_HPP
#define SU_CORE_SCENE_ANIMATION_ANIMATION_HPP

#include "scene/entity/keyframe.hpp"
#include <vector>
#include <cstddef>

namespace scene::animation {

class Animation {

public:

	void init(uint32_t count);

	void push_back(const entity::Keyframe& keyframe);

	void tick(float time_slice);
	void seek(float time);

	const entity::Keyframe& beginning() const;

	const entity::Keyframe& interpolated_frame() const;

private:

    float current_time_;

	uint32_t current_frame_;

	std::vector<entity::Keyframe> keyframes_;

	entity::Keyframe interpolated_frame_;
};

}

#endif
