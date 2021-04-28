#include "sky_material_base.hpp"
#include "base/math/vector3.inl"
#include "sky.hpp"

namespace procedural::sky {

using namespace scene::material;

Material::Material(Sky* sky)
    : scene::material::Material(
          Sampler_settings(Sampler_settings::Filter::Linear, Sampler_settings::Address::Repeat,
                           Sampler_settings::Address::Clamp),
          false),
      sky_(sky) {
    properties_.set(Property::Pure_emissive);
}

void Material::set_sky(Sky* sky) {
    sky_ = sky;
}

}  // namespace procedural::sky
