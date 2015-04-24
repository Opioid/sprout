#pragma once

#include <vector>

namespace scene {

struct Keyframe;

class Animation {
public:

	void reserve(size_t count);

	void push_back(const Keyframe& keyframe);

private:

	std::vector<Keyframe> keyframes_;
};

}
