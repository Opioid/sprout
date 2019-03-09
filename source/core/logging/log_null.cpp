#include "log_null.hpp"

namespace logging {

void Null::post(Type /*type*/, std::string const& /*text*/) {}

void Null::post(Type /*type*/, std::string const& /*text*/, std::string const& /*a*/) {}

void Null::push(Type /*type*/, std::string const& /*text*/) {}

void Null::push(Type /*type*/, std::string const& /*text*/, std::string const& /*a*/) {}

void Null::internal_post(Type /*type*/, std::string const& /*text*/) {}

}  // namespace logging
