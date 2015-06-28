#include "progress_sink_null.hpp"

namespace progress {

void Null::start(size_t /*resolution*/) {}

void Null::end() {}

void Null::tick() {}

}

