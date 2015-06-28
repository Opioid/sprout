#include "chrono.hpp"

namespace chrono {

float duration_to_seconds(std::chrono::high_resolution_clock::duration duration) {
	std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
	return static_cast<float>(milliseconds.count()) / 1000.f;
}

}
