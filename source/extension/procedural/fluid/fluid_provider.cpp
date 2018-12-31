#include "fluid_provider.hpp"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_float_1.hpp"
#include "core/image/typed_image.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "fluid_material.hpp"

namespace procedural::fluid {

using namespace scene;
using namespace scene::material;

static Provider provider;

void init(scene::Loader& loader) {
    provider.set_scene_loader(loader);

    loader.register_extension_provider("Fluid", &provider);
}

Provider::~Provider() noexcept {}

void Provider::set_scene_loader(Loader& loader) noexcept {
    scene_loader_ = &loader;
}

entity::Entity* Provider::create_extension(json::Value const& /*extension_value*/, Scene& scene,
                                           resource::Manager& /*manager*/) noexcept {
    using namespace image;

    int3 const viz_dimensions(320);

    auto target = std::make_shared<Float1>(Image::Description(Image::Type::Float1, viz_dimensions));

    auto texture = std::make_shared<texture::Float1>(target);

    auto material = std::make_shared<Material>(Sampler_settings(), Texture_adapter(texture));

    material->set_attenuation(float3(0.8f), float3(0.5f), 0.0005f);
    material->set_emission(float3(0.f));
    material->set_anisotropy(0.f);

    scene.add_material(material);

    prop::Prop* prop = scene.create_prop(scene_loader_->cube(), {material});

    prop->set_visibility(true, true, true);

    return prop;
}

}  // namespace procedural::fluid
