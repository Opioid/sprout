#include "base/platform/platform.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_std_out.hpp"
#include "core/rendering/rendering_driver_finalframe.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.inl"
#include "core/scene/scene_loader.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/take/take.hpp"
#include "core/take/take_loader.hpp"
#include "extension/procedural/sky/sky_provider.hpp"
#include "sprout.h"

using namespace scene;

using Shape_ptr = resource::Resource_ptr<shape::Shape>;

struct Engine {
    Engine() noexcept
        : threads(thread::Pool::num_threads(0)),
          resources(threads),
          image_resources(resources.register_provider(image_provider)),
          texture_provider(false),
          texture_resources(resources.register_provider(texture_provider)),
          material_provider(false),
          material_resources(resources.register_provider(material_provider)),
          shape_resources(resources.register_provider(mesh_provider)),
          scene_loader(resources, material_provider.create_fallback_material()),
          scene(scene_loader.null_shape(), shape_resources, material_resources, texture_resources),
          max_sample_size(material_provider.max_sample_size()) {}

    thread::Pool threads;

    resource::Manager resources;

    image::Provider                   image_provider;
    std::vector<image::Image*> const& image_resources;

    image::texture::Provider                     texture_provider;
    std::vector<image::texture::Texture*> const& texture_resources;

    material::Provider                      material_provider;
    std::vector<material::Material*> const& material_resources;

    shape::triangle::Provider         mesh_provider;
    std::vector<shape::Shape*> const& shape_resources;

    scene::Loader scene_loader;

    Scene scene;

    take::Take take;

    uint32_t const max_sample_size;
};

static Engine* engine = nullptr;

char const* su_platform_revision() noexcept {
    return platform::revision().c_str();
}

int32_t su_init() noexcept {
    engine = new Engine;

    logging::init(logging::Type::Std_out);

    procedural::sky::init(engine->scene_loader, engine->material_provider);

    return 1;
}

int32_t su_release() noexcept {
    delete engine;
    engine = nullptr;

    return 1;
}

int32_t su_load_take(char* const string) noexcept {
    if (!engine) {
        return 0;
    }

    bool success = true;

    std::string take_name;

    std::string take(string);

    {
        bool const is_json = string::is_json(take);

        auto stream = is_json ? file::Stream_ptr(new std::stringstream(take))
                              : engine->resources.filesystem().read_stream(take, take_name);

        if (!stream || !take::Loader::load(engine->take, *stream, take_name, engine->scene,
                                           engine->resources)) {
            logging::error("Loading take %S: ", string);
            success = false;
        }
    }

    if (success && !engine->scene_loader.load(engine->take.scene_filename, take_name, engine->take,
                                              engine->scene)) {
        logging::error("Loading scene %S: ", engine->take.scene_filename);
        success = false;
    }

    return success ? 1 : 0;
}

uint32_t su_create_prop(uint32_t shape, uint32_t num_materials,
                        uint32_t const* materials) noexcept {
    if (!engine) {
        return prop::Null;
    }

    auto const shape_ptr = engine->resources.get<shape::Shape>(shape);

    if (!shape_ptr) {
        return prop::Null;
    }

    uint32_t const num_expected_materials = shape_ptr.ptr->num_materials();

    auto& materials_buffer = engine->scene_loader.materials_buffer();

    materials_buffer.reserve(num_expected_materials);

    for (uint32_t i = 0, len = std::min(num_expected_materials, num_materials); i < len; ++i) {
        auto const material_ptr = engine->resources.get<material::Material>(materials[i]);

        if (!material_ptr) {
            continue;
        }

        materials_buffer.push_back(material_ptr);
    }

    if (1 == materials_buffer.size() && 1.f == materials_buffer[0].ptr->ior()) {
    } else {
        while (materials_buffer.size() < num_expected_materials) {
            materials_buffer.push_back(engine->scene_loader.fallback_material());
        }
    }

    return engine->scene.create_prop(shape_ptr, materials_buffer.data());
}

int32_t su_create_light(uint32_t prop) noexcept {
    if (!engine || engine->scene.num_props() <= prop) {
        return 0;
    }

	engine->scene_loader.create_light(prop, engine->scene);

	return 1;
}

int32_t su_prop_set_transformation(uint32_t prop, float const* transformation) noexcept {
    if (!engine || engine->scene.num_props() <= prop) {
        return 0;
    }

    float4x4 const m(transformation);

    float3x3 r;

    math::Transformation t;

    decompose(m, r, t.scale, t.position);

    t.rotation = quaternion::create(r);

    engine->scene.prop_set_world_transformation(prop, t);

    return 1;
}

int32_t su_render() noexcept {
    if (!engine) {
        return 0;
    }

    progress::Std_out progressor;

    if (engine->take.view.camera) {
        rendering::Driver_finalframe driver(engine->take, engine->scene, engine->threads,
                                            engine->max_sample_size, progressor);

        driver.render(engine->take.exporters);
    }

    return 1;
}

int square(int i) noexcept {
    return i * i;
}
