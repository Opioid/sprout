#include "dummy.hpp"
#include "base/math/vector4.inl"

namespace scene {
namespace entity {

void Dummy::set_parameters(json::Value const& /*parameters*/) noexcept {}

void Dummy::on_set_transformation() noexcept {}

}  // namespace entity
}  // namespace scene
