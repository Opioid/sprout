#include "log_null.hpp"

namespace logging {

void Null::post(Type /*type*/, std::string_view /*text*/) {}

void Null::post(Type /*type*/, std::string const& /*text*/, std::string const& /*a*/) {}

}  // namespace logging
