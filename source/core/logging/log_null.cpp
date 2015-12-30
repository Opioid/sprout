#include "log_null.hpp"

namespace logging {

void Null::post(Type /*type*/, const std::string& /*text*/) {}

}
