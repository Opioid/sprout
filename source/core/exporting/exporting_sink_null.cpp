#include "exporting_sink_null.hpp"
#include "base/math/vector4.inl"

namespace exporting {

Null::Null() {}

Null::~Null() {}

void Null::write(const image::Float4& /*image*/, uint32_t /*frame*/, thread::Pool& /*pool*/) {}

}  // namespace exporting
