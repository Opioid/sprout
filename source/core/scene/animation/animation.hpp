#pragma once

#include "scene/entity/keyframe.hpp"
#include <vector>
#include <cstddef>

namespace scene {

namespace animation {

class Animation {
public:

    void init(size_t count);

	void push_back(const entity::Keyframe& keyframe);

	void tick(float time_slice);

	void beginning(math::transformation& t) const;

	void current_frame(math::transformation& t) const;

private:

    float current_time_;

    size_t current_frame_;

	std::vector<entity::Keyframe> keyframes_;

	math::transformation interpolated_frame_;
};

}}
