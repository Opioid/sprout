#include "take_loader.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector.hpp"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"
#include "exporting/exporting_sink_ffmpeg.hpp"
#include "exporting/exporting_sink_image_sequence.hpp"
#include "exporting/exporting_sink_null.hpp"
#include "exporting/exporting_sink_statistics.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/encoding/rgbe/rgbe_writer.hpp"
#include "image/texture/texture.hpp"
#include "logging/logging.hpp"
#include "rendering/integrator/surface/ao.hpp"
#include "rendering/integrator/surface/debug.hpp"
#include "rendering/integrator/surface/lighttracer.hpp"
#include "rendering/integrator/surface/pathtracer.hpp"
#include "rendering/integrator/surface/pathtracer_dl.hpp"
#include "rendering/integrator/surface/pathtracer_mis.hpp"
#include "rendering/integrator/surface/whitted.hpp"
#include "rendering/integrator/volume/emission.hpp"
#include "rendering/integrator/volume/tracking_multi.hpp"
#include "rendering/integrator/volume/tracking_single.hpp"
#include "rendering/postprocessor/postprocessor_backplate.hpp"
#include "rendering/postprocessor/postprocessor_bloom.hpp"
#include "rendering/postprocessor/postprocessor_glare.hpp"
#include "rendering/postprocessor/postprocessor_glare2.hpp"
#include "rendering/postprocessor/postprocessor_glare3.hpp"
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
#include "scene/scene.hpp"
#include "take.hpp"

namespace take {

using Scene               = scene::Scene;
using Sensor_filter       = rendering::sensor::filter::Filter;
using Sensor_ptr          = rendering::sensor::Sensor*;
using Surface_factory_ptr = rendering::integrator::surface::Factory*;
using Volume_factory_ptr  = rendering::integrator::volume::Factory*;
using Postprocessor_ptr   = rendering::postprocessor::Postprocessor*;
using Light_sampling      = rendering::integrator::Light_sampling;

static void load_camera(json::Value const& camera_value, Take& take, Scene& scene);

template <typename Base>
static Sensor_ptr make_filtered_sensor(int2 dimensions, float exposure, float3 const& clamp_max,
                                       Sensor_filter const* filter);

static Sensor_ptr load_sensor(json::Value const& sensor_value, int2 dimensions);

static Sensor_filter const* load_filter(json::Value const& filter_value);

static sampler::Factory* load_sampler_factory(json::Value const& sampler_value,
                                              uint32_t           num_workers,
                                              uint32_t&          num_samples_per_pixel);

static void load_integrator_factories(json::Value const& integrator_value, uint32_t num_workers,
                                      Take& take);

static Surface_factory_ptr load_surface_integrator_factory(json::Value const& integrator_value,
                                                           Settings const&    settings,
                                                           uint32_t           num_workers);

static Volume_factory_ptr load_volume_integrator_factory(json::Value const& integrator_value,
                                                         Settings const&    settings,
                                                         uint32_t           num_workers);

static void load_photon_settings(json::Value const& value, Photon_settings& settings);

static void load_postprocessors(json::Value const& pp_value, resource::Manager& manager,
                                Take& take);

static Postprocessor_ptr load_tonemapper(json::Value const& tonemapper_value);

static bool peek_stereoscopic(json::Value const& parameters_value);

static std::vector<exporting::Sink*> load_exporters(json::Value const& exporter_value,
                                                    const View&        view);

static void load_settings(json::Value const& settings_value, Settings& settings);

static void load_light_sampling(json::Value const& parent_value, Light_sampling& sampling);

void Loader::load(Take& take, std::istream& stream, Scene& scene, resource::Manager& manager) {
    uint32_t const num_threads = manager.thread_pool().num_threads();

    auto root = json::parse(stream);

    json::Value const* postprocessors_value = nullptr;
    json::Value const* exporter_value       = nullptr;

    for (auto& n : root->GetObject()) {
        if ("camera" == n.name) {
            load_camera(n.value, take, scene);
        } else if ("export" == n.name) {
            exporter_value = &n.value;
        } else if ("start_frame" == n.name) {
            take.view.start_frame = json::read_uint(n.value);
        } else if ("num_frames" == n.name) {
            take.view.num_frames = json::read_uint(n.value);
        } else if ("integrator" == n.name) {
            load_integrator_factories(n.value, num_threads, take);
        } else if ("postprocessors" == n.name) {
            postprocessors_value = &n.value;
        } else if ("sampler" == n.name) {
            take.sampler_factory = load_sampler_factory(n.value, num_threads,
                                                        take.view.num_samples_per_pixel);
        } else if ("scene" == n.name) {
            take.scene_filename = n.value.GetString();
        } else if ("settings" == n.name) {
            load_settings(n.value, take.settings);
        }
    }

    if (take.scene_filename.empty()) {
        throw std::runtime_error("No reference to scene included");
    }

    if (take.view.camera) {
        if (postprocessors_value) {
            load_postprocessors(*postprocessors_value, manager, take);
        }

        if (exporter_value) {
            take.exporters = load_exporters(*exporter_value, take.view);
        }

        if (take.exporters.empty()) {
            auto const d = take.view.camera->sensor().dimensions();

            bool const error_diffusion = false;

            using namespace image;

            Writer* writer = new encoding::png::Writer(d, error_diffusion);

            take.exporters.push_back(new exporting::Image_sequence("output_", writer));

            logging::warning("No valid exporter was specified, defaulting to PNG writer.");
        }
    }

    if (!take.sampler_factory) {
        take.sampler_factory = new sampler::Random_factory(num_threads);

        logging::warning("No valid sampler was specified, defaulting to Random sampler.");
    }

    using namespace rendering::integrator;

    if (!take.surface_integrator_factory) {
        Light_sampling const light_sampling{Light_sampling::Strategy::Single, 1};

        uint32_t const num_samples = 1;
        uint32_t const min_bounces = 4;
        uint32_t const max_bounces = 8;

        float const path_continuation_probability = 0.9f;

        bool const enable_caustics = false;

        take.surface_integrator_factory = new surface::Pathtracer_MIS_factory(
            take.settings, num_threads, num_samples, min_bounces, max_bounces,
            path_continuation_probability, light_sampling, enable_caustics);

        logging::warning("No valid surface integrator specified, defaulting to PTMIS.");
    }

    if (!take.volume_integrator_factory) {
        take.volume_integrator_factory = new volume::Tracking_multi_factory(take.settings,
                                                                            num_threads);

        logging::warning("No valid volume integrator specified, defaulting to Tracking MS.");
    }

    take.view.init(manager.thread_pool());
}

static void load_camera(json::Value const& camera_value, Take& take, Scene& scene) {
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
        throw std::runtime_error("Empty camera object");
    }

    math::Transformation transformation{float3(0.f), float3(1.f), math::quaternion::identity()};

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
            throw std::runtime_error("Sensor resolution must be greater than zero");
        }
    } else {
        throw std::runtime_error("No sensor configuration included");
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
        throw std::runtime_error("Camera type \"" + type_name + "\" not recognized");
    }

    if (parameters_value) {
        camera->set_parameters(*parameters_value);
    }

    if (sensor_value) {
        auto sensor = load_sensor(*sensor_value, camera->sensor_dimensions());

        camera->set_sensor(sensor);
    }

    if (animation_value) {
        if (auto animation = scene::animation::load(*animation_value, transformation, scene);
            animation) {
            scene.create_animation_stage(camera, animation);
        }
    } else {
        camera->allocate_local_frame();
        camera->propagate_frame_allocation();
        camera->set_transformation(transformation);
    }

    take.view.camera = camera;
}

template <typename Base>
static Sensor_ptr make_filtered_sensor(int2 dimensions, float exposure, float3 const& clamp_max,
                                       Sensor_filter const* filter) {
    using namespace rendering::sensor;

    bool const clamp = !math::any_negative(clamp_max);

    if (clamp) {
        if (filter->radius() <= 1.f) {
            return new Filtered_1p0<Base, clamp::Clamp>(dimensions, exposure,
                                                        clamp::Clamp(clamp_max), filter);
        }

        if (filter->radius() <= 2.f) {
            return new Filtered_2p0<Base, clamp::Clamp>(dimensions, exposure,
                                                        clamp::Clamp(clamp_max), filter);
        }

        return new Filtered_inf<Base, clamp::Clamp>(dimensions, exposure, clamp::Clamp(clamp_max),
                                                    filter);
    }

    if (filter->radius() <= 1.f) {
        return new Filtered_1p0<Base, clamp::Identity>(dimensions, exposure, clamp::Identity(),
                                                       filter);
    }

    if (filter->radius() <= 2.f) {
        return new Filtered_2p0<Base, clamp::Identity>(dimensions, exposure, clamp::Identity(),
                                                       filter);
    }

    return new Filtered_inf<Base, clamp::Identity>(dimensions, exposure, clamp::Identity(), filter);
}

static Sensor_ptr load_sensor(json::Value const& sensor_value, int2 dimensions) {
    using namespace rendering::sensor;

    bool alpha_transparency = false;

    float exposure = 0.f;

    float3 clamp_max(-1.f);

    filter::Filter const* filter = nullptr;

    for (auto& n : sensor_value.GetObject()) {
        if ("alpha_transparency" == n.name) {
            alpha_transparency = json::read_bool(n.value);
        } else if ("exposure" == n.name) {
            exposure = json::read_float(n.value);
        } else if ("clamp" == n.name) {
            clamp_max = json::read_float3(n.value);
        } else if ("filter" == n.name) {
            filter = load_filter(n.value);
        }
    }

    if (filter) {
        if (alpha_transparency) {
            return make_filtered_sensor<Transparent>(dimensions, exposure, clamp_max, filter);
        }

        return make_filtered_sensor<Opaque>(dimensions, exposure, clamp_max, filter);
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

static Sensor_filter const* load_filter(json::Value const& filter_value) {
    using namespace rendering::sensor::filter;

    for (auto& n : filter_value.GetObject()) {
        if ("Gaussian" == n.name) {
            float const radius = json::read_float(n.value, "radius", 1.f);
            float const alpha  = json::read_float(n.value, "alpha", 1.8f);

            return new Gaussian(radius, alpha);
        } else if ("Mitchell" == n.name) {
            float const radius = json::read_float(n.value, "radius", 2.f);
            float const b      = json::read_float(n.value, "b", 1.f / 3.f);
            float const c      = json::read_float(n.value, "c", 1.f / 3.f);

            return new Mitchell(radius, b, c);
        }
    }

    logging::warning(
        "A reconstruction filter with unknonw type was declared. "
        "Not using any filter.");

    return nullptr;
}

static sampler::Factory* load_sampler_factory(json::Value const& sampler_value,
                                              uint32_t           num_workers,
                                              uint32_t&          num_samples_per_pixel) {
    for (auto& n : sampler_value.GetObject()) {
        num_samples_per_pixel = json::read_uint(n.value, "samples_per_pixel");

        if ("Uniform" == n.name) {
            num_samples_per_pixel = 1;
            return new sampler::Uniform_factory(num_workers);
        } else if ("Random" == n.name) {
            return new sampler::Random_factory(num_workers);
        } else if ("RD" == n.name) {
            return new sampler::RD_factory(num_workers);
        } else if ("Hammersley" == n.name) {
            return new sampler::Hammersley_factory(num_workers);
        } else if ("Golden_ratio" == n.name) {
            return new sampler::Golden_ratio_factory(num_workers);
        } else if ("LD" == n.name) {
            return new sampler::LD_factory(num_workers);
        }
    }

    return nullptr;
}

static void load_integrator_factories(json::Value const& integrator_value, uint32_t num_workers,
                                      Take& take) {
    for (auto& n : integrator_value.GetObject()) {
        if ("surface" == n.name) {
            take.surface_integrator_factory = load_surface_integrator_factory(
                n.value, take.settings, num_workers);
        } else if ("volume" == n.name) {
            take.volume_integrator_factory = load_volume_integrator_factory(n.value, take.settings,
                                                                            num_workers);
        } else if ("photon" == n.name) {
            load_photon_settings(n.value, take.photon_settings);
        }
    }
}

static Surface_factory_ptr load_surface_integrator_factory(json::Value const& integrator_value,
                                                           Settings const&    settings,
                                                           uint32_t           num_workers) {
    using namespace rendering::integrator::surface;

    uint32_t default_min_bounces = 4;
    uint32_t default_max_bounces = 8;

    Light_sampling light_sampling{Light_sampling::Strategy::All, 1};

    float default_path_continuation_probability = 0.9f;

    bool default_caustics = true;

    for (auto& n : integrator_value.GetObject()) {
        if ("AO" == n.name) {
            uint32_t const num_samples = json::read_uint(n.value, "num_samples", 1);
            float const    radius      = json::read_float(n.value, "radius", 1.f);
            return new AO_factory(settings, num_workers, num_samples, radius);
        } else if ("Whitted" == n.name) {
            uint32_t const num_light_samples = json::read_uint(n.value, "num_light_samples",
                                                               light_sampling.num_samples);

            return new Whitted_factory(settings, num_workers, num_light_samples);
        } else if ("LT" == n.name) {
            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            float const path_continuation_probability = json::read_float(
                n.value, "path_continuation_probability", default_path_continuation_probability);

            return new Lighttracer_factory(settings, num_workers, min_bounces, max_bounces,
                                           path_continuation_probability);
        } else if ("PT" == n.name) {
            uint32_t const num_samples = json::read_uint(n.value, "num_samples", 1);

            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            float const path_continuation_probability = json::read_float(
                n.value, "path_continuation_probability", default_path_continuation_probability);

            bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

            return new Pathtracer_factory(settings, num_workers, num_samples, min_bounces,
                                          max_bounces, path_continuation_probability,
                                          enable_caustics);
        } else if ("PTDL" == n.name) {
            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            float const path_continuation_probability = json::read_float(
                n.value, "path_continuation_probability", default_path_continuation_probability);

            uint32_t const num_light_samples = json::read_uint(n.value, "num_light_samples",
                                                               light_sampling.num_samples);

            bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

            return new Pathtracer_DL_factory(settings, num_workers, min_bounces, max_bounces,
                                             path_continuation_probability, num_light_samples,
                                             enable_caustics);
        } else if ("PTMIS" == n.name) {
            uint32_t const num_samples = json::read_uint(n.value, "num_samples", 1);

            uint32_t const min_bounces = json::read_uint(n.value, "min_bounces",
                                                         default_min_bounces);

            uint32_t const max_bounces = json::read_uint(n.value, "max_bounces",
                                                         default_max_bounces);

            float const path_continuation_probability = json::read_float(
                n.value, "path_continuation_probability", default_path_continuation_probability);

            load_light_sampling(n.value, light_sampling);

            bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

            return new Pathtracer_MIS_factory(settings, num_workers, num_samples, min_bounces,
                                              max_bounces, path_continuation_probability,
                                              light_sampling, enable_caustics);
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

            return new Debug_factory(settings, num_workers, vector);
        }
    }

    return nullptr;
}

static Volume_factory_ptr load_volume_integrator_factory(json::Value const& integrator_value,
                                                         Settings const&    settings,
                                                         uint32_t           num_workers) {
    using namespace rendering::integrator::volume;

    for (auto& n : integrator_value.GetObject()) {
        if ("Emission" == n.name) {
            float const step_size = json::read_float(n.value, "step_size", 1.f);

            return new Emission_factory(settings, num_workers, step_size);
        } else if ("Tracking" == n.name) {
            bool const multiple_scattering = json::read_bool(n.value, "multiple_scattering", true);

            uint2 const sr_range = uint2(
                json::read_int2(n.value, "similarity_relation_range", int2(16, 64)));

            using Volumetric_material = scene::material::volumetric::Material;
            Volumetric_material::set_similarity_relation_range(sr_range[0], sr_range[1]);

            if (multiple_scattering) {
                return new Tracking_multi_factory(settings, num_workers);
            } else {
                return new Tracking_single_factory(settings, num_workers);
            }
        }
    }

    return nullptr;
}

static void load_photon_settings(json::Value const& value, Photon_settings& settings) {
    settings.num_photons          = json::read_uint(value, "num_photons", 0);
    settings.max_bounces          = json::read_uint(value, "max_bounces", 2);
    settings.iteration_threshold  = json::read_float(value, "iteration_threshold", 0.f);
    settings.search_radius        = json::read_float(value, "search_radius", 0.01f);
    settings.merge_radius         = json::read_float(value, "merge_radius", 0.0025f);
    settings.coarse_search_radius = json::read_float(value, "coarse_search_radius", 0.1f);
    settings.indirect_photons     = json::read_bool(value, "indirect_photons", false);
    settings.separate_indirect    = json::read_bool(value, "separate_indirect", true);
    settings.full_light_path      = json::read_bool(value, "full_light_path", false);
}

static void load_postprocessors(json::Value const& pp_value, resource::Manager& manager,
                                Take& take) {
    if (!pp_value.IsArray()) {
        return;
    }

    using namespace rendering::postprocessor;

    auto& pipeline = take.view.pipeline;

    pipeline.reserve(pp_value.Size());

    for (auto const& pp : pp_value.GetArray()) {
        auto const n = pp.MemberBegin();

        if ("tonemapper" == n->name) {
            pipeline.add(load_tonemapper(n->value));
        } else if ("Backplate" == n->name) {
            std::string const name = json::read_string(n->value, "file");

            auto backplate = manager.load<image::texture::Texture>(name);

            if (take.view.camera &&
                backplate->dimensions_2() != take.view.camera->sensor_dimensions()) {
                logging::warning("Not using backplate \"" + name +
                                 "\", "
                                 "because resolution doesn't match sensor resolution.");
                continue;
            }

            pipeline.add(new Backplate(backplate));
        } else if ("Bloom" == n->name) {
            float const angle     = json::read_float(n->value, "angle", 0.05f);
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
        } else if ("Glare2" == n->name) {
            Glare2::Adaption adaption = Glare2::Adaption::Mesopic;

            std::string const adaption_name = json::read_string(n->value, "adaption");
            if ("Scotopic" == adaption_name) {
                adaption = Glare2::Adaption::Scotopic;
            } else if ("Mesopic" == adaption_name) {
                adaption = Glare2::Adaption::Mesopic;
            } else if ("Photopic" == adaption_name) {
                adaption = Glare2::Adaption::Photopic;
            }

            float const threshold = json::read_float(n->value, "threshold", 2.f);
            float const intensity = json::read_float(n->value, "intensity", 1.f);

            pipeline.add(new Glare2(adaption, threshold, intensity));
        } else if ("Glare3" == n->name) {
            Glare3::Adaption adaption = Glare3::Adaption::Mesopic;

            std::string const adaption_name = json::read_string(n->value, "adaption");
            if ("Scotopic" == adaption_name) {
                adaption = Glare3::Adaption::Scotopic;
            } else if ("Mesopic" == adaption_name) {
                adaption = Glare3::Adaption::Mesopic;
            } else if ("Photopic" == adaption_name) {
                adaption = Glare3::Adaption::Photopic;
            }

            float const threshold = json::read_float(n->value, "threshold", 2.f);
            float const intensity = json::read_float(n->value, "intensity", 1.f);

            pipeline.add(new Glare3(adaption, threshold, intensity));
        }
    }
}

static Postprocessor_ptr load_tonemapper(json::Value const& tonemapper_value) {
    using namespace rendering::postprocessor::tonemapping;

    for (auto& n : tonemapper_value.GetObject()) {
        if ("ACES" == n.name) {
            float const hdr_max = json::read_float(n.value, "hdr_max", 1.f);

            return new Aces(hdr_max);
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

static bool peek_stereoscopic(json::Value const& parameters_value) {
    auto const export_node = parameters_value.FindMember("stereo");
    if (parameters_value.MemberEnd() == export_node) {
        return false;
    }

    return true;
}

static std::vector<exporting::Sink*> load_exporters(json::Value const& exporter_value,
                                                    const View&        view) {
    if (!view.camera) {
        return {};
    }

    auto const& camera = *view.camera;

    std::vector<exporting::Sink*> exporters;

    for (auto& n : exporter_value.GetObject()) {
        if ("Image" == n.name) {
            std::string const format = json::read_string(n.value, "format", "PNG");

            image::Writer* writer;

            if ("RGBE" == format) {
                writer = new image::encoding::rgbe::Writer();
            } else {
                bool const transparent_sensor = camera.sensor().has_alpha_transparency();

                bool const error_diffusion = json::read_bool(n.value, "error_diffusion", false);

                if (view.pipeline.has_alpha_transparency(transparent_sensor)) {
                    writer = new image::encoding::png::Writer_alpha(camera.sensor().dimensions(),
                                                                    error_diffusion);
                } else {
                    writer = new image::encoding::png::Writer(camera.sensor().dimensions(),
                                                              error_diffusion);
                }
            }

            exporters.push_back(new exporting::Image_sequence("output_", writer));
        } else if ("Movie" == n.name) {
            uint32_t framerate = json::read_uint(n.value, "framerate");

            if (0 == framerate) {
                framerate = static_cast<uint32_t>(1.f / camera.frame_step() + 0.5f);
            }

            bool const error_diffusion = json::read_bool(n.value, "error_diffusion", false);

            exporters.push_back(new exporting::Ffmpeg("output", camera.sensor().dimensions(),
                                                      error_diffusion, framerate));
        } else if ("Null" == n.name) {
            exporters.push_back(new exporting::Null);
        } else if ("Statistics" == n.name) {
            exporters.push_back(new exporting::Statistics);
        }
    }

    return exporters;
}

static void load_settings(json::Value const& settings_value, Settings& settings) {
    for (auto& n : settings_value.GetObject()) {
        if ("ray_offset_factor" == n.name) {
            settings.ray_offset_factor = json::read_float(n.value);
        }
    }
}

static void load_light_sampling(json::Value const& parent_value, Light_sampling& sampling) {
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
