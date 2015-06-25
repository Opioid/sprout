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

	void reserve(size_t count);

	void push_back(const entity::Keyframe& keyframe);

private:

	std::vector<entity::Keyframe> keyframes_;
};

}}
