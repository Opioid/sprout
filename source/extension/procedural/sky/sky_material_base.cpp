#include "sky_material_base.hpp"
#include "base/math/vector3.inl"
#include "sky_model.hpp"

namespace procedural {
namespace sky {

using namespace scene::material;

Material::Material(Model& model)
    : scene::material::Material(
          Sampler_settings(Sampler_settings::Filter::Linear, Sampler_settings::Address::Repeat,
                           Sampler_settings::Address::Clamp),
          false),
      model_(model) {}

}  // namespace sky
}  // namespace procedural
