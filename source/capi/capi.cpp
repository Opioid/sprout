#include "base/encoding/encoding.inl"
#include "base/json/json.hpp"
#include "base/memory/array.inl"
#include "base/platform/platform.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.hpp"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/log.hpp"
#include "core/logging/logging.hpp"
#include "core/progress/progress_sink.hpp"
#include "core/rendering/rendering_driver.hpp"
#include "core/resource/resource_manager.inl"
#include "core/sampler/sampler_golden_ratio.hpp"
#include "core/sampler/sampler_random.hpp"
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
    void start(uint32_t resolution) final {
        if (start_) {
            start_(resolution);
        }
    }

    void tick() final {
        if (tick_) {
            tick_();
        }
    }

    Progress_start start_ = nullptr;
    Progress_tick  tick_  = nullptr;
};

}  // namespace progress

struct Engine {
    Engine(bool progressive)
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
          driver(threads, progressor),
          frame(0),
          frame_iteration(0),
          progressive(progressive),
          valid(false) {}
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

    rendering::Driver driver;

    uint32_t frame;
    uint32_t frame_iteration;

    bool const progressive;

    bool valid;
};

static Engine* engine = nullptr;

#define ASSERT_ENGINE(RESULT) \
    if (!engine) {            \
        return RESULT;        \
    }

#define ASSERT_PARSE(STRING, RESULT)            \
    rapidjson::Document root;                   \
    root.Parse(STRING);                         \
    if (root.HasParseError()) {                 \
        logging::error(json::read_error(root)); \
        return RESULT;                          \
    }

char const* su_platform_revision() {
    return platform::revision();
}

int32_t su_init(bool progressive) {
    if (engine) {
        return -1;
    }

    engine = new Engine(progressive);

    procedural::sky::init(engine->scene_loader, engine->material_provider);

    return 0;
}

int32_t su_release() {
    delete engine;
    engine = nullptr;

    return 0;
}

int32_t su_mount(char const* folder) {
    ASSERT_ENGINE(-1)

    engine->resources.filesystem().push_mount(folder);

    return 0;
}

int32_t su_clear() {
    ASSERT_ENGINE(-1)

    engine->take.clear();
    engine->scene.clear();

    engine->valid = false;

    return 0;
}

int32_t su_load_take(char const* string) {
    ASSERT_ENGINE(-1)

    bool success = true;

    std::string take_name;

    std::string const take(string);

    {
        bool const is_json = string::is_json(take);

        auto stream = is_json ? engine->resources.filesystem().string_stream(take)
                              : engine->resources.filesystem().read_stream(take, take_name);

        if (!stream || !take::Loader::load(engine->take, *stream, take_name, 0xFFFFFFFF,
                                           engine->progressive, engine->scene, engine->resources)) {
            logging::error("Loading take %S: ", string);
            success = false;
        }

        engine->valid = success && engine->take.view.valid();
    }

    if (success && !engine->scene_loader.load(engine->take.scene_filename, take_name, engine->take,
                                              engine->scene)) {
        logging::error("Loading scene %S: ", engine->take.scene_filename);
        success = false;
    }

    return success ? 0 : -2;
}

int32_t su_create_defaults() {
    ASSERT_ENGINE(-1)

    uint32_t const num_workers = engine->resources.threads().num_threads();

    take::Loader::set_default_integrators(num_workers, engine->progressive, engine->take.view);

    take::Loader::set_default_exporter(engine->take);

    scene::camera::Camera const* camera = engine->take.view.camera;

    engine->scene.calculate_num_interpolation_frames(camera->frame_step(),
                                                     camera->frame_duration());

    engine->valid = engine->take.view.valid();

    return 0;
}

uint32_t su_create_camera(char const* string) {
    ASSERT_ENGINE(prop::Null)

    ASSERT_PARSE(string, prop::Null)

    if (auto camera = take::Loader::load_camera(root, &engine->scene); camera) {
        engine->take.view.clear();

        engine->take.view.camera = camera;

        engine->valid = engine->take.view.valid();

        return camera->entity();
    }

    return prop::Null;
}

uint32_t su_create_camera_perspective(uint32_t width, uint32_t height, float fov) {
    ASSERT_ENGINE(prop::Null)

    engine->take.view.clear();

    camera::Perspective* camera = new camera::Perspective();

    int2 const resolution(width, height);
    camera->set_resolution(resolution, int4(int2(0), resolution));

    camera->set_fov(fov);

    float const radius = 1.f;
    float const alpha  = 1.8f;

    using namespace rendering::sensor;

    filter::Gaussian filter(radius, alpha);

    Sensor* sensor = new Filtered_1p0<Opaque, clamp::Identity, filter::Gaussian>(clamp::Identity(),
                                                                                 std::move(filter));

    camera->set_sensor(sensor);

    uint32_t const prop_id = engine->scene.create_entity();

    camera->init(prop_id);

    engine->take.view.camera = camera;

    engine->valid = engine->take.view.valid();

    return prop_id;
}

int32_t su_camera_set_resolution(uint32_t width, uint32_t height) {
    ASSERT_ENGINE(-1)

    scene::camera::Camera* camera = engine->take.view.camera;

    if (!camera) {
        return -2;
    }

    int2 const resolution(width, height);
    camera->set_resolution(resolution, int4(int2(0), resolution));

    return 0;
}

int32_t su_create_sampler(uint32_t num_samples) {
    ASSERT_ENGINE(-1)

    engine->take.view.num_samples_per_pixel = num_samples;

    if (!engine->take.view.samplers) {
        uint32_t const num_workers = engine->resources.threads().num_threads();

        if (engine->progressive) {
            engine->take.view.samplers = new sampler::Random_pool(num_workers);
        } else {
            engine->take.view.samplers = new sampler::Golden_ratio_pool(num_workers);
        }
    }

    engine->valid = engine->take.view.valid();

    return 0;
}

int32_t su_create_integrators(char const* string) {
    ASSERT_ENGINE(-1)

    ASSERT_PARSE(string, 0)

    uint32_t const num_workers = engine->resources.threads().num_threads();

    take::Loader::load_integrators(root, num_workers, false, engine->take.view);

    engine->valid = engine->take.view.valid();

    return 0;
}

uint32_t su_create_image(uint32_t pixel_type, uint32_t num_channels, uint32_t width,
                         uint32_t height, uint32_t depth, uint32_t num_elements, uint32_t stride,
                         char const* data) {
    ASSERT_ENGINE(resource::Null)

    int3 const dimensions(width, height, std::max(depth, 1u));

    using Description = image::Provider::Description;

    Description desc{Description::Pixel_type(pixel_type), num_channels, dimensions,
                     std::max(int32_t(num_elements), 0),  stride,       data};

    void const* desc_data = reinterpret_cast<void const*>(&desc);

    return engine->resources.load<image::Image>("", desc_data).id;
}

uint32_t su_create_material(char const* string) {
    ASSERT_ENGINE(resource::Null)

    ASSERT_PARSE(string, resource::Null)

    void const* data = reinterpret_cast<void const*>(&root);

    auto const material = engine->resources.load<material::Material>("", data);

    if (!material) {
        logging::error(string);
        return resource::Null;
    }

    material.ptr->commit(engine->threads, engine->scene);

    return material.id;
}

uint32_t su_create_material_from_file(char const* filename) {
    ASSERT_ENGINE(resource::Null)

    auto const material = engine->resources.load<material::Material>(filename);

    if (!material) {
        logging::error("");
        return resource::Null;
    }

    return material.id;
}

uint32_t su_create_triangle_mesh_async(uint32_t num_triangles, uint32_t num_vertices,
                                       uint32_t positions_stride, float const* positions,
                                       uint32_t normals_stride, float const* normals,
                                       uint32_t tangents_stride, float const* tangents,
                                       uint32_t uvs_stride, float const* uvs,
                                       uint32_t const* indices, uint32_t num_parts,
                                       uint32_t const* parts) {
    ASSERT_ENGINE(resource::Null)

    using Description = shape::triangle::Provider::Description;

    Description const desc{num_triangles,
                           num_vertices,
                           positions_stride,
                           normals_stride,
                           tangents_stride,
                           uvs_stride,
                           num_parts,
                           positions,
                           normals,
                           tangents,
                           uvs,
                           indices,
                           parts};

    void const* desc_data = reinterpret_cast<void const*>(&desc);

    return engine->resources.load<shape::Shape>("", desc_data).id;
}

uint32_t su_create_triangle_mesh_from_file(char const* filename) {
    ASSERT_ENGINE(resource::Null)

    auto const mesh = engine->resources.load<shape::Shape>(filename);

    if (!mesh) {
        logging::error("");
        return resource::Null;
    }

    return mesh.id;
}

uint32_t su_create_prop(uint32_t shape, uint32_t num_materials, uint32_t const* materials) {
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

int32_t su_create_light(uint32_t entity) {
    ASSERT_ENGINE(-1)

    if (engine->scene.num_props() <= entity) {
        return -2;
    }

    engine->scene_loader.create_light(entity, engine->scene);

    return 0;
}

uint32_t su_camera_entity() {
    ASSERT_ENGINE(prop::Null)

    if (!engine->take.view.camera) {
        return prop::Null;
    }

    return engine->take.view.camera->entity();
}

int32_t su_camera_sensor_dimensions(int32_t* dimensions) {
    ASSERT_ENGINE(-1)

    if (!engine->take.view.camera) {
        return -2;
    }

    int2 const d = engine->take.view.camera->sensor_dimensions();

    dimensions[0] = d[0];
    dimensions[1] = d[1];

    return 0;
}

int32_t su_entity_allocate_frames(uint32_t entity) {
    ASSERT_ENGINE(-1)

    if (engine->scene.num_props() <= entity) {
        return -2;
    }

    engine->scene.prop_allocate_frames(entity, true);

    return 0;
}

int32_t su_entity_transformation(uint32_t entity, float* transformation) {
    ASSERT_ENGINE(-1)

    if (engine->scene.num_props() <= entity) {
        return -2;
    }

    float4x4 const m = engine->scene.prop_world_transformation(entity).object_to_world();

    std::copy(m.r[0].v, &m.r[0].v[0] + 16, transformation);

    return 0;
}

int32_t su_entity_set_transformation(uint32_t entity, float const* transformation) {
    ASSERT_ENGINE(-1)

    if (engine->scene.num_props() <= entity) {
        return -2;
    }

    float4x4 const m(transformation);

    float3x3 r;

    Transformation t;

    decompose(m, r, t.scale, t.position);

    t.rotation = quaternion::create(r);

    engine->scene.prop_set_world_transformation(entity, t);

    return 0;
}

int32_t su_entity_set_transformation_frame(uint32_t entity, uint32_t frame,
                                           float const* transformation) {
    ASSERT_ENGINE(-1)

    if (engine->scene.num_props() <= entity || engine->scene.num_interpolation_frames() <= frame) {
        return -2;
    }

    float4x4 const m(transformation);

    float3x3 r;

    math::Transformation t;

    decompose(m, r, t.scale, t.position);

    t.rotation = quaternion::create(r);

    engine->scene.prop_set_frame(entity, frame, entity::Keyframe{t});

    return 0;
}

int32_t su_render() {
    ASSERT_ENGINE(-1)

    engine->threads.wait_async();

    if (!engine->valid) {
        return -2;
    }

    engine->driver.init(engine->take.view, engine->scene, engine->progressive);

    engine->driver.render(engine->take.exporters);

    return 0;
}

int32_t su_render_frame(uint32_t frame) {
    ASSERT_ENGINE(-1)

    engine->threads.wait_async();

    if (!engine->valid) {
        return -2;
    }

    engine->driver.init(engine->take.view, engine->scene, engine->progressive);

    engine->driver.render(frame);

    return 0;
}

int32_t su_export_frame(uint32_t frame) {
    ASSERT_ENGINE(-1)

    engine->driver.export_frame(frame, engine->take.exporters);

    return 0;
}

int32_t su_start_render_frame(uint32_t frame) {
    ASSERT_ENGINE(-1)

    engine->threads.wait_async();

    if (!engine->valid) {
        return -2;
    }

    engine->frame = frame;

    engine->frame_iteration = 0;

    engine->driver.init(engine->take.view, engine->scene, engine->progressive);

    engine->driver.start_frame(frame);

    return 0;
}

int32_t su_render_iteration() {
    ASSERT_ENGINE(-1)

    if (!engine->valid) {
        return -2;
    }

    engine->driver.render(engine->frame, engine->frame_iteration);

    ++engine->frame_iteration;

    return 0;
}

int32_t su_copy_framebuffer(uint32_t type, uint32_t width, uint32_t height, uint32_t num_channels,
                            uint8_t* destination) {
    ASSERT_ENGINE(-1)

    image::Float4 const& buffer = engine->driver.target();

    auto const d = buffer.description().dimensions();

    if (SU_UINT8 == type && 3 == num_channels) {
        struct Parameters {
            image::Float4 const& source;

            byte3* target;

            int32_t source_width;
            int32_t target_width;
        };

        Parameters parameters{buffer, reinterpret_cast<byte3*>(destination), d[0],
                              std::min(d[0], int32_t(width))};

        engine->threads.run_range(
            [&parameters](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
                image::Float4 const& source = parameters.source;

                byte3* target = parameters.target;

                int32_t const source_width = parameters.source_width;

                for (int32_t y = begin; y < end; ++y) {
                    int32_t i = y * source_width;

                    for (int32_t x = 0, width = parameters.target_width; x < width; ++x, ++i) {
                        float3 const color = spectrum::linear_to_gamma_sRGB(source.at(i).xyz());

                        target[i] = encoding::float_to_unorm(color);
                    }
                }
            },
            0, std::min(d[1], int32_t(height)));

        return 0;
    }

    if (SU_FLOAT32 == type && 4 == num_channels) {
        uint32_t const len = std::min(width * height, uint32_t(d[0] * d[1]));

        float const* raw = reinterpret_cast<float const*>(buffer.data());

        float* target = reinterpret_cast<float*>(destination);

        std::copy(raw, raw + 4 * len, target);
    }

    return -2;
}

namespace logging {

class C : public Log {
  public:
    C(Post post) : post_(post) {}

  private:
    void internal_post(Type type, std::string const& text) final {
        post_(uint32_t(type), text.c_str());
    }

    Post post_;
};

}  // namespace logging

int32_t su_register_log(Post post) {
    if (!post) {
        return -1;
    }

    logging::init(new logging::C(post));

    return 0;
}

int32_t su_register_progress(Progress_start start, Progress_tick tick) {
    ASSERT_ENGINE(-1)

    engine->progressor.start_ = start;
    engine->progressor.tick_  = tick;

    return 0;
}
