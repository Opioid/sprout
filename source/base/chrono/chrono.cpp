#include "chrono.hpp"

namespace chrono {

float duration_to_seconds(std::chrono::high_resolution_clock::duration duration) {
    auto const milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return float(milliseconds.count()) / 1000.f;
}

float seconds_since(std::chrono::high_resolution_clock::time_point time_point) {
    return duration_to_seconds(std::chrono::high_resolution_clock::now() - time_point);
}

}  // namespace chrono
