#include "exporting_sink_null.hpp"

namespace exporting {

Null::Null() {}

Null::~Null() {}

void Null::write(const image::Image& /*image*/, thread::Pool& /*pool*/) {}

}

