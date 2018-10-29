#ifndef SU_BASE_CHRONO_CHRONO_HPP
#define SU_BASE_CHRONO_CHRONO_HPP

#include <chrono>

namespace chrono {

float duration_to_seconds(std::chrono::high_resolution_clock::duration duration);

float seconds_since(std::chrono::high_resolution_clock::time_point time_point);

}  // namespace chrono

#endif
