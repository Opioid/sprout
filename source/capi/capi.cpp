#include "base/json/json.hpp"
#include "base/math/interpolated_function_1d.inl"
#include "base/memory/array.inl"
#include "base/platform/platform.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/log.hpp"
#include "core/logging/logging.hpp"
#include "core/progress/progress_sink.hpp"
#include "core/rendering/rendering_driver_finalframe.hpp"
#include "core/resource/resource_manager.inl"
#include "core/sampler/sampler.inl"
#include "core/sampler/sampler_golden_ratio.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/scene/camera/camera_perspective.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.inl"
#include "core/scene/scene_loader.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/take/take.hpp"
#include "core/take/take_loader.hpp"
#include "extension/procedural/sky/sky_provider.hpp"
#include "rendering/sensor/clamp.inl"
#include "rendering/sensor/filter/sensor_gaussian.hpp"
#include "rendering/sensor/filtered.inl"
#include "rendering/sensor/opaque.hpp"
#include "sprout.h"

using namespace scene;

using Shape_ptr = resource::Resource_ptr<shape::Shape>;

namespace progress {

class C : public Sink {
  public:
    void start(uint32_t resolution) noexcept override final {
        if (start_) {
            start_(resolution);
        }
    }

    void tick() noexcept override final {
        if (tick_) {
            tick_();
        }
    }

    Progress_start start_ = nullptr;
    Progress_tick  tick_  = nullptr;
};

}  // namespace progress

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
          driver(threads, material_provider.max_sample_size(), progressor) {}

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

    progress::C progressor;

    rendering::Driver_finalframe driver;
};

static Engine* engine = nullptr;

#define ASSERT_ENGINE(RESULT) \
    if (!engine) {            \
        return RESULT;        \
    }

#define ASSERT_PARSE(STRING, RESULT)               \
    std::string error;                             \
    auto        root = json::parse(STRING, error); \
    if (!root) {                                   \
        logging::error(error);                     \
        return RESULT;                             \
    }

char const* su_platform_revision() noexcept {
    return platform::revision().c_str();
}

int32_t su_init() noexcept {
    if (engine) {
        return -1;
    }

    engine = new Engine;

    procedural::sky::init(engine->scene_loader, engine->material_provider);

    return 0;
}

int32_t su_release() noexcept {
    delete engine;
    engine = nullptr;

    return 0;
}

int32_t su_mount(char const* folder) noexcept {
    ASSERT_ENGINE(-1)

    engine->resources.filesystem().push_mount(folder);

    return 0;
}

int32_t su_load_take(char const* string) noexcept {
    ASSERT_ENGINE(-1)

    bool success = true;

    std::string take_name;

    std::string const take(string);

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

    return success ? 0 : -2;
}

int32_t su_create_defaults() noexcept {
    ASSERT_ENGINE(-1)

    uint32_t const num_workers = engine->resources.threads().num_threads();

    take::Loader::set_default_integrators(num_workers, engine->take);

    take::Loader::set_default_exporter(engine->take);

    return 0;
}

uint32_t su_create_camera(char const* string) noexcept {
    ASSERT_ENGINE(prop::Null)

    ASSERT_PARSE(string, prop::Null)

    if (auto camera = take::Loader::load_camera(*root, engine->scene); camera) {
        engine->take.view.clear();

        engine->take.view.camera = camera;

        return camera->entity();
    }

    return prop::Null;
}

uint32_t su_create_camera_perspective(uint32_t width, uint32_t height, float fov) noexcept {
    ASSERT_ENGINE(prop::Null)

    engine->take.view.clear();

    int2 const resolution(width, height);

    camera::Perspective* camera = new camera::Perspective(resolution);

    camera->set_fov(fov);

    float const radius = 1.f;
    float const alpha  = 1.8f;

    using namespace rendering::sensor;

    filter::Gaussian filter(radius, alpha);

    Sensor* sensor = new Filtered_1p0<Opaque, clamp::Identity, filter::Gaussian>(
        camera->sensor_dimensions(), 0.f, clamp::Identity(), std::move(filter));

    camera->set_sensor(sensor);

    uint32_t const prop_id = engine->scene.create_entity();

    camera->init(prop_id);

    engine->take.view.camera = camera;

    return prop_id;
}

int32_t su_create_sampler(uint32_t num_samples) noexcept {
    ASSERT_ENGINE(-1)

    engine->take.view.num_samples_per_pixel = num_samples;

    if (!engine->take.samplers) {
        uint32_t const num_workers = engine->resources.threads().num_threads();

        engine->take.samplers = new sampler::Golden_ratio_pool(num_workers);
    }

    return 0;
}

int32_t su_create_integrators(char const* string) noexcept {
    ASSERT_ENGINE(-1)

    ASSERT_PARSE(string, 0)

    uint32_t const num_workers = engine->resources.threads().num_threads();

    take::Loader::load_integrators(*root, num_workers, engine->take);

    return 0;
}

uint32_t su_create_image(uint32_t pixel_type, uint32_t num_channels, uint32_t width,
                         uint32_t height, uint32_t depth, uint32_t num_elements, char const* data,
                         uint32_t stride) noexcept {
    ASSERT_ENGINE(resource::Null)

    int3 const dimensions(width, height, std::max(depth, 1u));

    using Description = image::Provider::Description;

    Description desc{Description::Pixel_type(pixel_type), num_channels, dimensions,
                     std::max(int32_t(num_elements), 0),  stride,       data};

    void const* desc_data = reinterpret_cast<void const*>(&desc);

    return engine->resources.load<image::Image>("", desc_data).id;
}

uint32_t su_create_material(char const* string) noexcept {
    ASSERT_ENGINE(resource::Null)

    ASSERT_PARSE(string, resource::Null)

    void const* data = reinterpret_cast<void const*>(&(*root));

    auto const material = engine->resources.load<material::Material>("", data);

    if (!material) {
        logging::error(string);
        return resource::Null;
    }

    material.ptr->compile(engine->threads, engine->scene);

    return material.id;
}

uint32_t su_create_material_from_file(char const* filename) noexcept {
    ASSERT_ENGINE(resource::Null)

    auto const material = engine->resources.load<material::Material>(filename);

    if (!material) {
        logging::error("");
        return resource::Null;
    }

    return material.id;
}

uint32_t su_create_triangle_mesh(uint32_t num_vertices, float const* positions,
                                 uint32_t positions_stride, float const* normals,
                                 uint32_t normals_stride, float const* tangents,
                                 uint32_t tangents_stride, float const* texture_coordinates,
                                 uint32_t texture_coordinates_stride, uint32_t num_indices,
                                 uint32_t const* indices, uint32_t num_parts,
                                 uint32_t const* parts) noexcept {
    ASSERT_ENGINE(resource::Null)

    using Description = shape::triangle::Provider::Description;

    Description const desc{num_vertices,
                           positions_stride,
                           normals_stride,
                           tangents_stride,
                           texture_coordinates_stride,
                           num_indices,
                           num_parts,
                           positions,
                           normals,
                           tangents,
                           texture_coordinates,
                           indices,
                           parts};

    void const* desc_data = reinterpret_cast<void const*>(&desc);

    return engine->resources.load<shape::Shape>("", desc_data).id;
}

uint32_t su_create_triangle_mesh_from_file(char const* filename) noexcept {
    ASSERT_ENGINE(resource::Null)

    auto const mesh = engine->resources.load<shape::Shape>(filename);

    if (!mesh) {
        logging::error("");
        return resource::Null;
    }

    return mesh.id;
}

uint32_t su_create_prop(uint32_t shape, uint32_t num_materials,
                        uint32_t const* materials) noexcept {
    ASSERT_ENGINE(prop::Null)

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

int32_t su_create_light(uint32_t entity) noexcept {
    ASSERT_ENGINE(-1)

    if (engine->scene.num_props() <= entity) {
        return 0;
    }

    engine->scene_loader.create_light(entity, engine->scene);

    return 1;
}

uint32_t su_camera_entity() noexcept {
    ASSERT_ENGINE(prop::Null)

    if (!engine->take.view.camera) {
        return prop::Null;
    }

    return engine->take.view.camera->entity();
}

int32_t su_entity_set_transformation(uint32_t entity, float const* transformation) noexcept {
    ASSERT_ENGINE(-1)

    if (engine->scene.num_props() <= entity) {
        return -1;
    }

    float4x4 const m(transformation);

    float3x3 r;

    math::Transformation t;

    decompose(m, r, t.scale, t.position);

    t.rotation = quaternion::create(r);

    engine->scene.prop_set_world_transformation(entity, t);

    return 0;
}

int32_t su_render() noexcept {
    ASSERT_ENGINE(-1)

    engine->threads.wait_async();

    if (!engine->take.view.camera || !engine->take.surface_integrators) {
        return -2;
    }

    engine->driver.init(engine->take, engine->scene);

    engine->driver.render(engine->take.exporters);

    return 0;
}

int32_t su_render_frame(uint32_t frame) noexcept {
    ASSERT_ENGINE(-1)

    engine->threads.wait_async();

    if (!engine->take.view.camera || !engine->take.surface_integrators) {
        return -2;
    }

    engine->driver.init(engine->take, engine->scene);

    engine->driver.render(frame, engine->take.exporters);

    return 0;
}

namespace logging {

class C : public Log {
  public:
    C(Post post) noexcept : post_(post) {}

  private:
    void internal_post(Type type, std::string const& text) noexcept override final {
        post_(uint32_t(type), text.c_str());
    }

    Post post_;
};

}  // namespace logging

int32_t su_register_log(Post post, bool verbose) noexcept {
    if (!post) {
        return -1;
    }

    logging::init(new logging::C(post));

    logging::set_verbose(verbose);

    return 0;
}

int32_t su_register_progress(Progress_start start, Progress_tick tick) noexcept {
    ASSERT_ENGINE(-1)

    engine->progressor.start_ = start;
    engine->progressor.tick_  = tick;

    return 0;
}
