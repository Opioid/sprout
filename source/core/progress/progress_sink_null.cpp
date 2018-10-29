#include "progress_sink_null.hpp"

namespace progress {

void Null::start(uint32_t /*resolution*/) {}

void Null::end() {}

void Null::tick() {}

}  // namespace progress
