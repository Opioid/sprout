#include "exporting_sink_null.hpp"

namespace exporting {

Null::Null() noexcept = default;

Null::~Null() noexcept = default;

void Null::write(image::Float4 const& /*image*/, uint32_t /*frame*/,
                 thread::Pool& /*threads*/) noexcept {}

}  // namespace exporting
