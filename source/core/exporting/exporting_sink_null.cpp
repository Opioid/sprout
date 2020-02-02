#include "exporting_sink_null.hpp"

namespace exporting {

Null::Null() = default;

Null::~Null() = default;

void Null::write(image::Float4 const& /*image*/, uint32_t /*frame*/, thread::Pool& /*threads*/) {}

}  // namespace exporting
