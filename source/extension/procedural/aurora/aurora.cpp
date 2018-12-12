#include "aurora.hpp"

namespace procedural::aurora {

bool Aurora::is_extension() const noexcept {
    return true;
}

void Aurora::set_parameters(json::Value const& /*parameters*/) noexcept {}

void Aurora::on_set_transformation() noexcept {}

}  // namespace procedural::aurora
