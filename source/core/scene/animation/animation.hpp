#pragma once

#include <vector>
#include <cstddef>

namespace scene {

namespace entity {

struct Keyframe;

}

namespace animation {

class Animation {
public:

    void init(size_t count);

	void push_back(const entity::Keyframe& keyframe);

    void beginning(entity::Keyframe& keyframe) const;

    void tick(float time_slice, entity::Keyframe& keyframe);

private:

    float current_time_;

    size_t current_frame_;

	std::vector<entity::Keyframe> keyframes_;
};

}}
