#include "log_null.hpp"

namespace logging {

void Null::post(Type /*type*/, std::string_view /*text*/) {}

}  // namespace logging
