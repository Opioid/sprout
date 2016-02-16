#include "exporting_sink_null.hpp"
#include "base/math/vector.inl"

namespace exporting {

Null::Null() {}

Null::~Null() {}

void Null::write(const image::Image_float_4& /*image*/, uint32_t /*frame*/, thread::Pool& /*pool*/) {}

}

