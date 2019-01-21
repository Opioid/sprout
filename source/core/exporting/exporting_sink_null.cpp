#include "exporting_sink_null.hpp"

namespace exporting {

Null::Null() noexcept {}

Null::~Null() noexcept {}

void Null::write(image::Float4 const& /*image*/, uint32_t /*frame*/,
                 thread::Pool& /*pool*/) noexcept {}

}  // namespace exporting
