#include "take_loader.hpp"
#include "base/json/json.hpp"
#include "base/math/interpolated_function_1d.inl"
#include "base/math/math.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector.hpp"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "exporting/exporting_sink_ffmpeg.hpp"
#include "exporting/exporting_sink_image_sequence.hpp"
#include "exporting/exporting_sink_null.hpp"
#include "exporting/exporting_sink_statistics.hpp"
#include "image/encoding/exr/exr_writer.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/encoding/rgbe/rgbe_writer.hpp"
#include "image/texture/texture.inl"
#include "logging/logging.hpp"
#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/integrator/surface/debug.hpp"
#include "rendering/integrator/surface/pathtracer.hpp"
#include "rendering/integrator/surface/pathtracer_dl.hpp"
#include "rendering/integrator/surface/pathtracer_mis.hpp"
#include "rendering/integrator/surface/pm.hpp"
#include "rendering/integrator/surface/whitted.hpp"
#include "rendering/integrator/volume/emission.hpp"
#include "rendering/integrator/volume/tracking_multi.hpp"
#include "rendering/integrator/volume/tracking_single.hpp"
#include "rendering/postprocessor/postprocessor_backplate.hpp"
#include "rendering/postprocessor/postprocessor_bloom.hpp"
#include "rendering/postprocessor/postprocessor_glare.hpp"
#include "rendering/postprocessor/tonemapping/aces.hpp"
#include "rendering/postprocessor/tonemapping/generic.hpp"
#include "rendering/postprocessor/tonemapping/identity.hpp"
#include "rendering/postprocessor/tonemapping/uncharted.hpp"
#include "rendering/sensor/clamp.inl"
#include "rendering/sensor/filter/sensor_gaussian.hpp"
#include "rendering/sensor/filter/sensor_mitchell.hpp"
#include "rendering/sensor/filtered.inl"
#include "rendering/sensor/opaque.hpp"
#include "rendering/sensor/transparent.hpp"
#include "rendering/sensor/unfiltered.inl"
#include "resource/resource_manager.inl"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_hammersley.hpp"
#include "sampler/sampler_ld.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_rd.hpp"
#include "sampler/sampler_uniform.hpp"
#include "scene/animation/animation_loader.hpp"
#include "scene/camera/camera_cubic.hpp"
#include "scene/camera/camera_cubic_stereoscopic.hpp"
#include "scene/camera/camera_hemispherical.hpp"
#include "scene/camera/camera_perspective.hpp"
#include "scene/camera/camera_perspective_stereoscopic.hpp"
#include "scene/camera/camera_spherical.hpp"
#include "scene/camera/camera_spherical_stereoscopic.hpp"
#include "scene/material/volumetric/volumetric_material.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene.inl"
#include "take.hpp"

namespace take {

using Scene          = scene::Scene;
using Camera         = scene::camera::Camera;
using Sensor_filter  = rendering::sensor::filter::Filter;
using Sensor         = rendering::sensor::Sensor;
using Surface_pool   = rendering::integrator::surface::Pool;
using Volume_pool    = rendering::integrator::volume::Pool;
using Particle_pool  = rendering::integrator::particle::Lighttracer_pool;
using Postprocessor  = rendering::postprocessor::Postprocessor;
using Light_sampling = rendering::integrator::Light_sampling;

static Sensor* load_sensor(json::Value const& sensor_value, int2 dimensions) noexcept;

static sampler::Pool* load_sampler_pool(json::Value const& sampler_value, uint32_t num_workers,
                                        bool progressive, uint32_t& num_samples_per_pixel) noexcept;

static Surface_pool* load_surface_integrator(json::Value const& integrator_value,
                                             uint32_t num_workers, bool progressive,
                                             bool lighttracer) noexcept;

static Volume_pool* load_volume_integrator(json::Value const& integrator_value,
                                           uint32_t num_workers, bool progressive) noexcept;

static Particle_pool* load_particle_integrator(json::Value const& integrator_value,
                                               uint32_t           num_workers,
                                               uint64_t&          num_particles) noexcept;

static void load_photon_settings(json::Value const& value, Photon_settings& settings) noexcept;

static Postprocessor* load_tonemapper(json::Value const& tonemapper_value) noexcept;

static bool peek_stereoscopic(json::Value const& parameters_value) noexcept;

static memory::Array<exporting::Sink*> load_exporters(json::Value const& exporter_value,
                                                      View const&        view) noexcept;

static void load_light_sampling(json::Value const& parent_value, Light_sampling& sampling) noexcept;

bool Loader::load(Take& take, std::istream& stream, std::string_view take_name, bool progressive,
                  Scene& scene, resource::Manager& resources) noexcept {
    uint32_t const num_threads = resources.threads().num_threads();

    std::string error;
    auto const  root = json::parse(stream, error);
    if (!root) {
        logging::push_error(error);
        return false;
    }

    json::Value const* postprocessors_value = nullptr;
    json::Value const* exporter_value       = nullptr;

    for (auto& n : root->GetObject()) {
        if ("camera" == n.name) {
            if (Camera* camera = load_camera(n.value, scene); camera) {
                take.view.camera = camera;
            } else {
                return false;
            }
        } else if ("export" == n.name) {
            exporter_value = &n.value;
        } else if ("start_frame" == n.name) {
            take.view.start_frame = json::read_uint(n.value);
        } else if ("num_frames" == n.name) {
            take.view.num_frames = json::read_uint(n.value);
        } else if ("integrator" == n.name) {
            load_integrators(n.value, num_threads, progressive, take);
        } else if ("post" == n.name || "postprocessors" == n.name) {
            postprocessors_value = &n.value;
        } else if ("sampler" == n.name) {
            take.samplers = load_sampler_pool(n.value, num_threads, progressive,
                                              take.view.num_samples_per_pixel);
        } else if ("scene" == n.name) {
            take.scene_filename = n.value.GetString();
        }
    }

    if (take.scene_filename.empty()) {
        logging::push_error("No reference to scene included.");
        return false;
    }

    if (take.view.camera) {
        if (postprocessors_value) {
            std::string_view const take_mount_folder = string::parent_directory(take_name);

            auto& filesystem = resources.filesystem();

            filesystem.push_mount(take_mount_folder);

            load_postprocessors(*postprocessors_value, resources, take.view.pipeline,
                                take.view.camera->sensor_dimensions());

            filesystem.pop_mount();
        }

        if (exporter_value) {
            take.exporters = load_exporters(*exporter_value, take.view);
        }

        set_default_exporter(take);
    }

    if (!take.samplers) {
        take.samplers = new sampler::Random_pool(num_threads);

        logging::warning("No valid sampler was specified, defaulting to Random sampler.");
    }

    set_default_integrators(num_threads, progressive, take);

    take.view.init(resources.threads());

    return true;
}

Camera* Loader::load_camera(json::Value const& camera_value, Scene& scene) noexcept {
    using namespace scene::camera;

    std::string type_name;

    json::Value const* type_value = nullptr;

    for (auto& n : camera_value.GetObject()) {
        type_name  = n.name.GetString();
        type_value = &n.value;

        if ("Cubic" == type_name) {
            break;
        }
    }

    if (!type_value) {
        // Can this happen at all!
        logging::error("Empty camera object");
        return nullptr;
    }

    math::Transformation transformation{float3(0.f), float3(1.f), quaternion::identity()};

    json::Value const* parameters_value = nullptr;
    json::Value const* animation_value  = nullptr;
    json::Value const* sensor_value     = nullptr;

    std::string layout_type;

    bool stereo = false;

    for (auto& n : type_value->GetObject()) {
        if ("parameters" == n.name) {
            parameters_value = &n.value;
            stereo           = peek_stereoscopic(n.value);
        } else if ("transformation" == n.name) {
            json::read_transformation(n.value, transformation);
        } else if ("animation" == n.name) {
            animation_value = &n.value;
        } else if ("sensor" == n.name) {
            sensor_value = &n.value;
        } else if ("layout" == n.name) {
            layout_type = json::read_string(n.value);
        }
    }

    int2 resolution;
    if (sensor_value) {
        resolution = json::read_int2(*sensor_value, "resolution", int2::identity());
        if (int2::identity() == resolution) {
            logging::error("Sensor resolution must be greater than zero");
            return nullptr;
        }
    } else {
        logging::error("No sensor configuration included");
        return nullptr;
    }

    Camera* camera;

    if ("Cubic" == type_name) {
        if (stereo) {
            Cubic_stereoscopic::Layout layout =
                Cubic_stereoscopic::Layout::rxlmxryrmyrzrmzlxlmxlylmylzlmz;

            if ("lxlmxlylmylzlmzrxrmxryrmyrzrmz" == layout_type) {
                layout = Cubic_stereoscopic::Layout::lxlmxlylmylzlmzrxrmxryrmyrzrmz;
            }

            camera = new Cubic_stereoscopic(layout, resolution);
        } else {
            Cubic::Layout layout = Cubic::Layout::xmxymyzmz;

            if ("xmxy_myzmz" == layout_type) {
                layout = Cubic::Layout::xmxy_myzmz;
            }

            camera = new Cubic(layout, resolution);
        }
    } else if ("Perspective" == type_name) {
        if (stereo) {
            camera = new Perspective_stereoscopic(resolution);
        } else {
            camera = new Perspective(resolution);
        }
    } else if ("Spherical" == type_name) {
        if (stereo) {
            camera = new Spherical_stereoscopic(resolution);
        } else {
            camera = new Spherical(resolution);
        }
    } else if ("Hemispherical" == type_name) {
        camera = new Hemispherical(resolution);
    } else {
        logging::error("Camera type \"" + type_name + "\" not recognized");
        return nullptr;
    }

    if (parameters_value) {
        camera->set_parameters(*parameters_value);
    }

    if (sensor_value) {
        auto sensor = load_sensor(*sensor_value, camera->sensor_dimensions());

        camera->set_sensor(sensor);
    }

    uint32_t const prop_id = scene.create_entity();

    camera->init(prop_id);

    if (animation_value) {
        if (auto animation = scene::animation::load(*animation_value, transformation, scene);
            animation) {
            scene.create_animation_stage(prop_id, animation);
        }
    } else {
        scene.prop_set_world_transformation(prop_id, transformation);
    }

    return camera;
}

template <typename Filter>
Filter load_filter(json::Value const& /*filter_value*/) noexcept;

template <>
rendering::sensor::filter::Gaussian load_filter(json::Value const& filter_value) noexcept {
    float radius = 1.f;
    float alpha  = 1.8f;

    for (auto& n : filter_value.GetObject()) {
        if ("Gaussian" == n.name) {
            radius = json::read_float(n.value, "radius", radius);
            alpha  = json::read_float(n.value, "alpha", alpha);

            break;
        }
    }

    return rendering::sensor::filter::Gaussian(radius, alpha);
}

template <>
rendering::sensor::filter::Mitchell load_filter(json::Value const& filter_value) noexcept {
    float radius = 2.f;
    float b      = 1.f / 3.f;
    float c      = 1.f / 3.f;

    for (auto& n : filter_value.GetObject()) {
        if ("Mitchell" == n.name) {
            radius = json::read_float(n.value, "radius", radius);
            b      = json::read_float(n.value, "b", b);
            c      = json::read_float(n.value, "c", c);

            break;
        }
    }

    return rendering::sensor::filter::Mitchell(radius, b, c);
}

template <typename Base, typename Filter>
static Sensor* make_filtered_sensor(int2 dimensions, float exposure, float3 const& clamp_max,
                                    json::Value const& filter_value) noexcept {
    using namespace rendering::sensor;

    bool const clamp = !math::any_negative(clamp_max);

    Filter filter = load_filter<Filter>(filter_value);

    if (clamp) {
        if (filter.radius() <= 1.f) {
            return new Filtered_1p0<Base, clamp::Clamp, Filter>(
                dimensions, exposure, clamp::Clamp(clamp_max), std::move(filter));
        }

        if (filter.radius() <= 2.f) {
            return new Filtered_2p0<Base, clamp::Clamp, Filter>(
                dimensions, exposure, clamp::Clamp(clamp_max), std::move(filter));
        }

        return new Filtered_inf<Base, clamp::Clamp, Filter>(
            dimensions, exposure, clamp::Clamp(clamp_max), std::move(filter));
    }

    if (filter.radius() <= 1.f) {
        return new Filtered_1p0<Base, clamp::Identity, Filter>(
            dimensions, exposure, clamp::Identity(), std::move(filter));
    }

    if (filter.radius() <= 2.f) {
        return new Filtered_2p0<Base, clamp::Identity, Filter>(
            dimensions, exposure, clamp::Identity(), std::move(filter));
    }

    return new Filtered_inf<Base, clamp::Identity, Filter>(dimensions, exposure, clamp::Identity(),
                                                           std::move(filter));
}

enum class Sensor_filter_type { Undefined, Gaussian, Mitchell };

static Sensor_filter_type read_filter_type(json::Value const& filter_value) noexcept {
    for (auto& n : filter_value.GetObject()) {
        if ("Gaussian" == n.name) {
            return Sensor_filter_type::Gaussian;
        } else if ("Mitchell" == n.name) {
            return Sensor_filter_type::Mitchell;
        }
    }

    logging::warning(
        "A reconstruction filter with unknonw type was declared. "
        "Not using any filter.");

    return Sensor_filter_type::Undefined;
}

static Sensor* load_sensor(json::Value const& sensor_value, int2 dimensions) noexcept {
    using namespace rendering::sensor;
    using namespace rendering::sensor::filter;

    bool alpha_transparency = false;

    float exposure = 0.f;

    float3 clamp_max(-1.f);

    json::Value const* filter_value = nullptr;

    Sensor_filter_type filter_type = Sensor_filter_type::Undefined;

    for (auto& n : sensor_value.GetObject()) {
        if ("alpha_transparency" == n.name) {
            alpha_transparency = json::read_bool(n.value);
        } else if ("exposure" == n.name) {
            exposure = json::read_float(n.value);
        } else if ("clamp" == n.name) {
            clamp_max = json::read_float3(n.value);
        } else if ("filter" == n.name) {
            filter_value = &n.value;
            filter_type  = read_filter_type(n.value);
        }
    }

    if (filter_value && Sensor_filter_type::Undefined != filter_type) {
        if (alpha_transparency) {
            if (Sensor_filter_type::Gaussian == filter_type) {
                return make_filtered_sensor<Transparent, Gaussian>(dimensions, exposure, clamp_max,
                                                                   *filter_value);
            }

            return make_filtered_sensor<Transparent, Mitchell>(dimensions, exposure, clamp_max,
                                                               *filter_value);
        }

        if (Sensor_filter_type::Gaussian == filter_type) {
            return make_filtered_sensor<Opaque, Gaussian>(dimensions, exposure, clamp_max,
                                                          *filter_value);
        }

        return make_filtered_sensor<Opaque, Mitchell>(dimensions, exposure, clamp_max,
                                                      *filter_value);
    }

    bool const clamp = !math::any_negative(clamp_max);

    if (alpha_transparency) {
        if (clamp) {
            return new Unfiltered<Transparent, clamp::Clamp>(dimensions, exposure,
                                                             clamp::Clamp(clamp_max));
        } else {
            return new Unfiltered<Transparent, clamp::Identity>(dimensions, exposure,
                                                                clamp::Identity());
        }
    }

    if (clamp) {
        return new Unfiltered<Opaque, clamp::Clamp>(dimensions, exposure, clamp::Clamp(clamp_max));
    }

    return new Unfiltered<Opaque, clamp::Identity>(dimensions, exposure, clamp::Identity());
}

static sampler::Pool* load_sampler_pool(json::Value const& sampler_value, uint32_t num_workers,
                                        bool      progressive,
                                        uint32_t& num_samples_per_pixel) noexcept {
    if (progressive) {
        num_samples_per_pixel = 1;
        return new sampler::Random_pool(num_workers);
    }

    for (auto& n : sampler_value.GetObject()) {
        num_samples_per_pixel = json::read_uint(n.value, "samples_per_pixel");

        if ("Uniform" == n.name) {
            num_samples_per_pixel = 1;
            return new sampler::Uniform_pool(num_workers);
        } else if ("Random" == n.name) {
            return new sampler::Random_pool(num_workers);
        } else if ("RD" == n.name) {
            return new sampler::RD_pool(num_workers);
        } else if ("Hammersley" == n.name) {
            return new sampler::Hammersley_pool(num_workers);
        } else if ("Golden_ratio" == n.name) {
            return new sampler::Golden_ratio_pool(num_workers);
        } else if ("LD" == n.name) {
            return new sampler::LD_pool(num_workers);
        }
    }

    return nullptr;
}

template <class T>
static inline void replace(T*& former, T* newer) noexcept {
    if (newer) {
        delete former;

        former = newer;
    }
}

void Loader::load_integrators(json::Value const& integrator_value, uint32_t num_workers,
                              bool progressive, Take& take) noexcept {
    json::Value::ConstMemberIterator const particle_node = integrator_value.FindMember("particle");

    if (integrator_value.MemberEnd() != particle_node) {
        take.lighttracers = load_particle_integrator(particle_node->value, num_workers,
                                                     take.view.num_particles);
    }

    for (auto& n : integrator_value.GetObject()) {
        if ("surface" == n.name) {
            replace(take.surface_integrators,
                    load_surface_integrator(n.value, num_workers, progressive,
                                            nullptr != take.lighttracers));
        } else if ("volume" == n.name) {
            replace(take.volume_integrators,
                    load_volume_integrator(n.value, num_workers, progressive));
        } else if ("photon" == n.name) {
            load_photon_settings(n.value, take.view.photon_settings);
        }
    }
}

static Surface_pool* load_surface_integrator(json::Value const& integrator_value,
                                             uint32_t num_workers, bool progressive,
                                             bool lighttracer) noexcept {
    using namespace rendering::integrator::surface;

    uint32_t const default_min_bounces = 4;
    uint32_t const default_max_bounces = 8;

    Light_sampling light_sampling{Light_sampling::Strategy::All, 1};

    bool const default_caustics = true;

    for (auto& n : integrator_value.GetObject()) {
        if ("AO" == n.name) {
            uint32_t const num_samples = json::read_uint(n.value, "num_samples", 1);

            float const radius = json::read_float(n.value, "radius", 1.f);

            return new AO_pool(num_workers, progressive, num_samples, radius);
        } else if ("Whitted" == n.name) {
            uint32_t const num_light_samples = json::read_uint(n.value, "num_light_samples",
                                                               light_sampling.num_samples);

            return new Whitted_pool(num_workers, num_light_samples);
        } else if ("PM" == n.name) {
            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            bool const photons_only_through_specular = /*true;  //*/ lighttracer;

            return new PM_pool(num_workers, progressive, min_bounces, max_bounces,
                               photons_only_through_specular);
        } else if ("PT" == n.name) {
            uint32_t const num_samples = json::read_uint(n.value, "num_samples", 1);

            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

            return new Pathtracer_pool(num_workers, progressive, num_samples, min_bounces,
                                       max_bounces, enable_caustics);
        } else if ("PTDL" == n.name) {
            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            uint32_t const num_light_samples = json::read_uint(n.value, "num_light_samples",
                                                               light_sampling.num_samples);

            bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

            return new Pathtracer_DL_pool(num_workers, progressive, min_bounces, max_bounces,
                                          num_light_samples, enable_caustics);
        } else if ("PTMIS" == n.name) {
            uint32_t const num_samples = json::read_uint(n.value, "num_samples", 1);

            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            load_light_sampling(n.value, light_sampling);

            bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics) &&
                                         !lighttracer;

            bool const photons_only_through_specular = lighttracer;

            return new Pathtracer_MIS_pool(num_workers, progressive, num_samples, min_bounces,
                                           max_bounces, light_sampling, enable_caustics,
                                           photons_only_through_specular);
        } else if ("Debug" == n.name) {
            auto vector = Debug::Settings::Vector::Shading_normal;

            std::string const vector_type = json::read_string(n.value, "vector");

            if ("Tangent" == vector_type) {
                vector = Debug::Settings::Vector::Tangent;
            } else if ("Bitangent" == vector_type) {
                vector = Debug::Settings::Vector::Bitangent;
            } else if ("Geometric_normal" == vector_type) {
                vector = Debug::Settings::Vector::Geometric_normal;
            } else if ("Shading_normal" == vector_type) {
                vector = Debug::Settings::Vector::Shading_normal;
            } else if ("UV" == vector_type) {
                vector = Debug::Settings::Vector::UV;
            }

            return new Debug_pool(num_workers, vector);
        }
    }

    return nullptr;
}

static Volume_pool* load_volume_integrator(json::Value const& integrator_value,
                                           uint32_t num_workers, bool progressive) noexcept {
    using namespace rendering::integrator::volume;

    for (auto& n : integrator_value.GetObject()) {
        if ("Emission" == n.name) {
            float const step_size = json::read_float(n.value, "step_size", 1.f);

            return new Emission_pool(num_workers, step_size);
        } else if ("Tracking" == n.name) {
            bool const multiple_scattering = json::read_bool(n.value, "multiple_scattering", true);

            uint2 const sr_range = uint2(
                json::read_int2(n.value, "similarity_relation_range", int2(16, 64)));

            using Volumetric_material = scene::material::volumetric::Material;
            Volumetric_material::set_similarity_relation_range(sr_range[0], sr_range[1]);

            if (multiple_scattering) {
                return new Tracking_multi_pool(num_workers);
            } else {
                return new Tracking_single_pool(num_workers, progressive);
            }
        }
    }

    return nullptr;
}

static Particle_pool* load_particle_integrator(json::Value const& integrator_value,
                                               uint32_t           num_workers,
                                               uint64_t&          num_particles) noexcept {
    using namespace rendering::integrator::particle;

    bool const indirect_caustics = json::read_bool(integrator_value, "indirect_caustics", true);
    bool const full_light_path   = json::read_bool(integrator_value, "full_light_path", false);

    uint32_t const max_bounces = json::read_uint(integrator_value, "max_bounces", 8);

    num_particles = json::read_uint64(integrator_value, "num_particles", 1280000);

    return new Lighttracer_pool(num_workers, 1, max_bounces, num_particles, indirect_caustics,
                                full_light_path);
}

void Loader::set_default_integrators(uint32_t num_workers, bool progressive, Take& take) noexcept {
    using namespace rendering::integrator;

    if (!take.surface_integrators) {
        Light_sampling const light_sampling{Light_sampling::Strategy::Single, 1};

        uint32_t const num_samples = 1;
        uint32_t const min_bounces = 4;
        uint32_t const max_bounces = 8;

        bool const enable_caustics = false;

        take.surface_integrators = new surface::Pathtracer_MIS_pool(
            num_workers, progressive, num_samples, min_bounces, max_bounces, light_sampling,
            enable_caustics, false);

        logging::warning("No valid surface integrator specified, defaulting to PTMIS.");
    }

    if (!take.volume_integrators) {
        take.volume_integrators = new volume::Tracking_multi_pool(num_workers);

        logging::warning("No valid volume integrator specified, defaulting to Tracking MS.");
    }
}

static void load_photon_settings(json::Value const& value, Photon_settings& settings) noexcept {
    settings.num_photons         = json::read_uint(value, "num_photons", 0);
    settings.max_bounces         = json::read_uint(value, "max_bounces", 4);
    settings.iteration_threshold = json::read_float(value, "iteration_threshold", 1.f);
    settings.search_radius       = json::read_float(value, "search_radius", 0.002f);
    settings.merge_radius        = json::read_float(value, "merge_radius", 0.001f);
    settings.indirect_photons    = json::read_bool(value, "indirect_photons", true);
    settings.full_light_path     = json::read_bool(value, "full_light_path", false);
}

void Loader::load_postprocessors(json::Value const& pp_value, resource::Manager& resources,
                                 Pipeline& pipeline, int2 dimensions) noexcept {
    if (!pp_value.IsArray()) {
        return;
    }

    using namespace rendering::postprocessor;

    pipeline.reserve(pp_value.Size());

    for (auto const& pp : pp_value.GetArray()) {
        auto const n = pp.MemberBegin();

        if ("tonemapper" == n->name) {
            pipeline.add(load_tonemapper(n->value));
        } else if ("Backplate" == n->name) {
            std::string const name = json::read_string(n->value, "file");

            auto backplate_res = resources.load<image::texture::Texture>(name);
            if (!backplate_res.ptr) {
                continue;
            }

            auto backplate = backplate_res.ptr;

            if (backplate->dimensions_2() != dimensions) {
                logging::warning(
                    "Not using backplate %S, "
                    "because resolution does not match sensor resolution.",
                    name);
                continue;
            }

            pipeline.add(new Backplate(backplate));
        } else if ("Bloom" == n->name) {
            float const angle     = json::read_float(n->value, "angle", 0.00002f);
            float const alpha     = json::read_float(n->value, "alpha", 0.005f);
            float const threshold = json::read_float(n->value, "threshold", 2.f);
            float const intensity = json::read_float(n->value, "intensity", 0.1f);

            pipeline.add(new Bloom(angle, alpha, threshold, intensity));
        } else if ("Glare" == n->name) {
            Glare::Adaption adaption = Glare::Adaption::Mesopic;

            std::string const adaption_name = json::read_string(n->value, "adaption");
            if ("Scotopic" == adaption_name) {
                adaption = Glare::Adaption::Scotopic;
            } else if ("Mesopic" == adaption_name) {
                adaption = Glare::Adaption::Mesopic;
            } else if ("Photopic" == adaption_name) {
                adaption = Glare::Adaption::Photopic;
            }

            float const threshold = json::read_float(n->value, "threshold", 2.f);
            float const intensity = json::read_float(n->value, "intensity", 1.f);

            pipeline.add(new Glare(adaption, threshold, intensity));
        } else {
            logging::warning("Unknown postprocessor \"" + std::string(n->name.GetString()) + "\"");
        }
    }
}

static Postprocessor* load_tonemapper(json::Value const& tonemapper_value) noexcept {
    using namespace rendering::postprocessor::tonemapping;

    for (auto& n : tonemapper_value.GetObject()) {
        if ("ACES" == n.name) {
            float const hdr_max = json::read_float(n.value, "hdr_max", 1.f);

            return new Aces(hdr_max);
        } else if ("ACES_MJP" == n.name) {
            return new Aces_MJP();
        } else if ("Generic" == n.name) {
            float const contrast = json::read_float(n.value, "contrast", 1.15f);
            float const shoulder = json::read_float(n.value, "shoulder", 0.99f);
            float const mid_in   = json::read_float(n.value, "mid_in", 0.18f);
            float const mid_out  = json::read_float(n.value, "mid_out", 0.18f);
            float const hdr_max  = json::read_float(n.value, "hdr_max", 1.f);

            return new Generic(contrast, shoulder, mid_in, mid_out, hdr_max);
        } else if ("Identity" == n.name) {
            return new Identity();
        } else if ("Uncharted" == n.name) {
            float const hdr_max = json::read_float(n.value, "hdr_max", 1.f);

            return new Uncharted(hdr_max);
        }
    }

    return nullptr;
}

static bool peek_stereoscopic(json::Value const& parameters_value) noexcept {
    auto const export_node = parameters_value.FindMember("stereo");
    if (parameters_value.MemberEnd() == export_node) {
        return false;
    }

    return true;
}

static memory::Array<exporting::Sink*> load_exporters(json::Value const& exporter_value,
                                                      View const&        view) noexcept {
    if (!view.camera) {
        return {};
    }

    auto const& camera = *view.camera;

    memory::Array<exporting::Sink*> exporters;
    exporters.reserve(exporter_value.GetObject().MemberCount());

    for (auto& n : exporter_value.GetObject()) {
        if ("Image" == n.name) {
            using namespace image::encoding;

            std::string const format = json::read_string(n.value, "format", "PNG");

            image::Writer* writer;

            if ("EXR" == format) {
                bool const transparent_sensor = camera.sensor().has_alpha_transparency();

                bool const alpha = view.pipeline.has_alpha_transparency(transparent_sensor);

                writer = new exr::Writer(alpha);
            } else if ("RGBE" == format) {
                writer = new rgbe::Writer();
            } else {
                bool const transparent_sensor = camera.sensor().has_alpha_transparency();

                bool const error_diffusion = json::read_bool(n.value, "error_diffusion", false);

                bool const pre_multiplied_alpha = json::read_bool(n.value, "pre_multiplied_alpha",
                                                                  true);

                if (view.pipeline.has_alpha_transparency(transparent_sensor)) {
                    writer = new png::Writer_alpha(error_diffusion, pre_multiplied_alpha);
                } else {
                    writer = new png::Writer(error_diffusion);
                }
            }

            exporters.push_back(new exporting::Image_sequence("output_", writer));
        } else if ("Movie" == n.name) {
            uint32_t framerate = json::read_uint(n.value, "framerate");

            if (0 == framerate) {
                framerate = uint32_t(1.f / camera.frame_step() + 0.5f);
            }

            bool const error_diffusion = json::read_bool(n.value, "error_diffusion", false);

            exporters.push_back(new exporting::Ffmpeg("output", camera.sensor().dimensions(),
                                                      error_diffusion, framerate));
        } else if ("Null" == n.name) {
            exporters.push_back(new exporting::Null);
        } else if ("Stats" == n.name || "Statistics" == n.name) {
            exporters.push_back(new exporting::Statistics);
        }
    }

    return exporters;
}

void Loader::set_default_exporter(Take& take) noexcept {
    if (take.exporters.empty()) {
        bool const error_diffusion = false;

        using namespace image;

        Writer* writer = new encoding::png::Writer(error_diffusion);

        take.exporters.reserve(1);
        take.exporters.push_back(new exporting::Image_sequence("output_", writer));

        logging::warning("No valid exporter was specified, defaulting to PNG writer.");
    }
}

static void load_light_sampling(json::Value const& parent_value,
                                Light_sampling&    sampling) noexcept {
    auto const light_sampling_node = parent_value.FindMember("light_sampling");
    if (parent_value.MemberEnd() == light_sampling_node) {
        return;
    }

    for (auto& n : light_sampling_node->value.GetObject()) {
        if ("strategy" == n.name) {
            std::string const strategy = json::read_string(n.value);

            if ("Single" == strategy) {
                sampling.strategy = Light_sampling::Strategy::Single;
            } else if ("All" == strategy) {
                sampling.strategy = Light_sampling::Strategy::All;
            }
        } else if ("num_samples" == n.name) {
            sampling.num_samples = json::read_uint(n.value);
        }
    }
}

}  // namespace take
