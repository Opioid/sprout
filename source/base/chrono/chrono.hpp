#pragma once

#include <chrono>

namespace chrono {

float duration_to_seconds(std::chrono::high_resolution_clock::duration duration);

}
