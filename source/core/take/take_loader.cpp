#include "take_loader.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/quaternion.inl"
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
#include "rendering/integrator/surface/pathtracer.hpp"
#include "rendering/integrator/surface/pathtracer_dl.hpp"
#include "rendering/integrator/surface/pathtracer_mis.hpp"
#include "rendering/integrator/surface/whitted.hpp"
#include "rendering/integrator/volume/aerial_perspective.hpp"
#include "rendering/integrator/volume/emission.hpp"
#include "rendering/integrator/volume/flow_vis.hpp"
#include "rendering/integrator/volume/ray_marching_single.hpp"
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
#include "rendering/sensor/filtered.inl"
#include "rendering/sensor/opaque.hpp"
#include "rendering/sensor/transparent.hpp"
#include "rendering/sensor/unfiltered.inl"
#include "resource/resource_manager.inl"
#include "sampler/sampler_ems.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_hammersley.hpp"
#include "sampler/sampler_ld.hpp"
#include "sampler/sampler_random.hpp"
#include "sampler/sampler_uniform.hpp"
#include "scene/animation/animation_loader.hpp"
#include "scene/camera/camera_cubic.hpp"
#include "scene/camera/camera_cubic_stereoscopic.hpp"
#include "scene/camera/camera_hemispherical.hpp"
#include "scene/camera/camera_perspective.hpp"
#include "scene/camera/camera_perspective_stereoscopic.hpp"
#include "scene/camera/camera_spherical.hpp"
#include "scene/camera/camera_spherical_stereoscopic.hpp"
#include "take.hpp"

namespace take {

std::unique_ptr<Take> Loader::load(std::istream& stream, resource::Manager& manager) {
  uint32_t const num_threads = manager.thread_pool().num_threads();

  auto root = json::parse(stream);

  auto take = std::make_unique<Take>();

  json::Value const* postprocessors_value = nullptr;
  json::Value const* exporter_value = nullptr;

  for (auto& n : root->GetObject()) {
    if ("camera" == n.name) {
      load_camera(n.value, *take);
    } else if ("export" == n.name) {
      exporter_value = &n.value;
    } else if ("start_frame" == n.name) {
      take->view.start_frame = json::read_uint(n.value);
    } else if ("num_frames" == n.name) {
      take->view.num_frames = json::read_uint(n.value);
    } else if ("integrator" == n.name) {
      load_integrator_factories(n.value, num_threads, *take);
    } else if ("postprocessors" == n.name) {
      postprocessors_value = &n.value;
    } else if ("sampler" == n.name) {
      take->sampler_factory =
          load_sampler_factory(n.value, num_threads, take->view.num_samples_per_pixel);
    } else if ("scene" == n.name) {
      take->scene_filename = n.value.GetString();
    } else if ("settings" == n.name) {
      load_settings(n.value, take->settings);
    }
  }

  if (take->scene_filename.empty()) {
    throw std::runtime_error("No reference to scene included");
  }

  if (take->view.camera) {
    if (postprocessors_value) {
      load_postprocessors(*postprocessors_value, manager, *take);
    }

    if (exporter_value) {
      take->exporters = load_exporters(*exporter_value, take->view);
    }

    if (take->exporters.empty()) {
      auto const d = take->view.camera->sensor().dimensions();

      std::unique_ptr<image::Writer> writer = std::make_unique<image::encoding::png::Writer>(d);

      take->exporters.push_back(
          std::make_unique<exporting::Image_sequence>("output_", std::move(writer)));

      logging::warning("No valid exporter was specified, defaulting to PNG writer.");
    }
  }

  if (!take->sampler_factory) {
    take->sampler_factory = std::make_shared<sampler::Random_factory>(num_threads);

    logging::warning("No valid sampler was specified, defaulting to Random sampler.");
  }

  using namespace rendering::integrator;

  if (!take->surface_integrator_factory) {
    Light_sampling const light_sampling{Light_sampling::Strategy::Single, 1};
    uint32_t const min_bounces = 4;
    uint32_t const max_bounces = 8;
    float const path_termination_probability = 0.9f;
    bool const enable_caustics = false;

    take->surface_integrator_factory = std::make_shared<surface::Pathtracer_MIS_factory>(
        take->settings, num_threads, min_bounces, max_bounces, path_termination_probability,
        light_sampling, enable_caustics);

    logging::warning("No valid surface integrator specified, defaulting to PTMIS.");
  }

  if (!take->volume_integrator_factory) {
    take->volume_integrator_factory =
        std::make_shared<volume::Tracking_multi_factory>(take->settings, num_threads);

    logging::warning("No valid volume integrator specified, defaulting to Tracking MS.");
  }

  take->view.init(manager.thread_pool());

  return take;
}

void Loader::load_camera(json::Value const& camera_value, Take& take) {
  using namespace scene::camera;

  std::string type_name;
  json::Value const* type_value = nullptr;

  for (auto& n : camera_value.GetObject()) {
    type_name = n.name.GetString();
    type_value = &n.value;

    if ("Cubic" == type_name) {
      break;
    }
  }

  if (!type_value) {
    // Can this happen at all!
    throw std::runtime_error("Empty camera object");
  }

  math::Transformation transformation{float3::identity(), float3(1.f),
                                      math::quaternion::identity()};

  json::Value const* parameters_value = nullptr;
  json::Value const* animation_value = nullptr;
  json::Value const* sensor_value = nullptr;

  std::string layout_type;
  bool stereo = false;

  for (auto& n : type_value->GetObject()) {
    if ("parameters" == n.name) {
      parameters_value = &n.value;
      stereo = peek_stereoscopic(n.value);
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

  if (animation_value) {
    take.camera_animation = scene::animation::load(*animation_value, transformation);
  }

  std::shared_ptr<Camera> camera;

  if ("Cubic" == type_name) {
    if (stereo) {
      Cubic_stereoscopic::Layout layout =
          Cubic_stereoscopic::Layout::rxlmxryrmyrzrmzlxlmxlylmylzlmz;

      if ("lxlmxlylmylzlmzrxrmxryrmyrzrmz" == layout_type) {
        layout = Cubic_stereoscopic::Layout::lxlmxlylmylzlmzrxrmxryrmyrzrmz;
      }

      camera = std::make_shared<Cubic_stereoscopic>(layout, resolution);
    } else {
      Cubic::Layout layout = Cubic::Layout::xmxymyzmz;

      if ("xmxy_myzmz" == layout_type) {
        layout = Cubic::Layout::xmxy_myzmz;
      }

      camera = std::make_shared<Cubic>(layout, resolution);
    }
  } else if ("Perspective" == type_name) {
    if (stereo) {
      camera = std::make_shared<Perspective_stereoscopic>(resolution);
    } else {
      camera = std::make_shared<Perspective>(resolution);
    }
  } else if ("Spherical" == type_name) {
    if (stereo) {
      camera = std::make_shared<Spherical_stereoscopic>(resolution);
    } else {
      camera = std::make_shared<Spherical>(resolution);
    }
  } else if ("Hemispherical" == type_name) {
    camera = std::make_shared<Hemispherical>(resolution);
  } else {
    throw std::runtime_error("Camera type \"" + type_name + "\" not recognized");
  }

  if (parameters_value) {
    camera->set_parameters(*parameters_value);
  }

  camera->set_transformation(transformation);

  if (sensor_value) {
    auto sensor = load_sensor(*sensor_value, camera->sensor_dimensions());

    camera->set_sensor(std::move(sensor));
  }

  take.view.camera = camera;
}

std::unique_ptr<rendering::sensor::Sensor> Loader::load_sensor(json::Value const& sensor_value,
                                                               int2 dimensions) {
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

  bool const clamp = !math::any_negative(clamp_max);

  if (filter) {
    if (alpha_transparency) {
      if (clamp) {
        return std::make_unique<Filtered<Transparent, clamp::Clamp>>(
            dimensions, exposure, clamp::Clamp(clamp_max), filter);
      } else {
        return std::make_unique<Filtered<Transparent, clamp::Identity>>(dimensions, exposure,
                                                                        clamp::Identity(), filter);
      }
    }

    if (clamp) {
      return std::make_unique<Filtered<Opaque, clamp::Clamp>>(dimensions, exposure,
                                                              clamp::Clamp(clamp_max), filter);
    } else {
      return std::make_unique<Filtered<Opaque, clamp::Identity>>(dimensions, exposure,
                                                                 clamp::Identity(), filter);
    }
  }

  if (alpha_transparency) {
    if (clamp) {
      return std::make_unique<Unfiltered<Transparent, clamp::Clamp>>(dimensions, exposure,
                                                                     clamp::Clamp(clamp_max));
    } else {
      return std::make_unique<Unfiltered<Transparent, clamp::Identity>>(dimensions, exposure,
                                                                        clamp::Identity());
    }
  }

  if (clamp) {
    return std::make_unique<Unfiltered<Opaque, clamp::Clamp>>(dimensions, exposure,
                                                              clamp::Clamp(clamp_max));
  }

  return std::make_unique<Unfiltered<Opaque, clamp::Identity>>(dimensions, exposure,
                                                               clamp::Identity());
}

const rendering::sensor::filter::Filter* Loader::load_filter(json::Value const& filter_value) {
  for (auto& n : filter_value.GetObject()) {
    if ("Gaussian" == n.name) {
      float const radius = json::read_float(n.value, "radius", 1.f);
      float const alpha = json::read_float(n.value, "alpha", 1.8f);

      return new rendering::sensor::filter::Gaussian(radius, alpha);
    }
  }

  logging::warning(
      "A reconstruction filter with unknonw type was declared. "
      "Not using any filter.");

  return nullptr;
}

std::shared_ptr<sampler::Factory> Loader::load_sampler_factory(json::Value const& sampler_value,
                                                               uint32_t num_workers,
                                                               uint32_t& num_samples_per_pixel) {
  for (auto& n : sampler_value.GetObject()) {
    num_samples_per_pixel = json::read_uint(n.value, "samples_per_pixel");

    if ("Uniform" == n.name) {
      num_samples_per_pixel = 1;
      return std::make_shared<sampler::Uniform_factory>(num_workers);
    } else if ("Random" == n.name) {
      return std::make_shared<sampler::Random_factory>(num_workers);
    } else if ("Hammersley" == n.name) {
      return std::make_shared<sampler::Hammersley_factory>(num_workers);
    } else if ("EMS" == n.name) {
      return std::make_shared<sampler::EMS_factory>(num_workers);
    } else if ("Golden_ratio" == n.name) {
      return std::make_shared<sampler::Golden_ratio_factory>(num_workers);
    } else if ("LD" == n.name) {
      return std::make_shared<sampler::LD_factory>(num_workers);
    }
  }

  return nullptr;
}

void Loader::load_integrator_factories(json::Value const& integrator_value, uint32_t num_workers,
                                       Take& take) {
  for (auto& n : integrator_value.GetObject()) {
    if ("surface" == n.name) {
      take.surface_integrator_factory =
          load_surface_integrator_factory(n.value, take.settings, num_workers);
    } else if ("volume" == n.name) {
      take.volume_integrator_factory =
          load_volume_integrator_factory(n.value, take.settings, num_workers);
    }
  }
}

std::shared_ptr<rendering::integrator::surface::Factory> Loader::load_surface_integrator_factory(
    json::Value const& integrator_value, Settings const& settings, uint32_t num_workers) {
  using namespace rendering::integrator;
  using namespace rendering::integrator::surface;

  uint32_t default_min_bounces = 4;
  uint32_t default_max_bounces = 8;
  Light_sampling light_sampling{Light_sampling::Strategy::All, 1};
  float default_path_termination_probability = 0.9f;
  bool default_caustics = true;

  for (auto& n : integrator_value.GetObject()) {
    if ("AO" == n.name) {
      uint32_t const num_samples = json::read_uint(n.value, "num_samples", 1);
      float const radius = json::read_float(n.value, "radius", 1.f);
      return std::make_shared<AO_factory>(settings, num_workers, num_samples, radius);
    } else if ("Whitted" == n.name) {
      uint32_t const num_light_samples =
          json::read_uint(n.value, "num_light_samples", light_sampling.num_samples);

      return std::make_shared<Whitted_factory>(settings, num_workers, num_light_samples);
    } else if ("PT" == n.name) {
      uint32_t const min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);

      uint32_t const max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

      float const path_termination_probability = json::read_float(
          n.value, "path_termination_probability", default_path_termination_probability);

      bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

      return std::make_shared<Pathtracer_factory>(settings, num_workers, min_bounces, max_bounces,
                                                  path_termination_probability, enable_caustics);
    } else if ("PTDL" == n.name) {
      uint32_t const min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);

      uint32_t const max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

      float const path_termination_probability = json::read_float(
          n.value, "path_termination_probability", default_path_termination_probability);

      uint32_t const num_light_samples =
          json::read_uint(n.value, "num_light_samples", light_sampling.num_samples);

      bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

      return std::make_shared<Pathtracer_DL_factory>(settings, num_workers, min_bounces,
                                                     max_bounces, path_termination_probability,
                                                     num_light_samples, enable_caustics);
    } else if ("PTMIS" == n.name) {
      uint32_t const min_bounces = json::read_uint(n.value, "min_bounces", default_min_bounces);

      uint32_t const max_bounces = json::read_uint(n.value, "max_bounces", default_max_bounces);

      float const path_termination_probability = json::read_float(
          n.value, "path_termination_probability", default_path_termination_probability);

      load_light_sampling(n.value, light_sampling);

      bool const enable_caustics = json::read_bool(n.value, "caustics", default_caustics);

      return std::make_shared<Pathtracer_MIS_factory>(settings, num_workers, min_bounces,
                                                      max_bounces, path_termination_probability,
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

      return std::make_shared<Debug_factory>(settings, num_workers, vector);
    }
  }

  return nullptr;
}

std::shared_ptr<rendering::integrator::volume::Factory> Loader::load_volume_integrator_factory(
    json::Value const& integrator_value, Settings const& settings, uint32_t num_workers) {
  using namespace rendering::integrator;
  using namespace rendering::integrator::volume;

  for (auto& n : integrator_value.GetObject()) {
    if ("Aerial_perspective" == n.name) {
      float const step_size = json::read_float(n.value, "step_size", 1.f);
      bool const shadows = json::read_bool(n.value, "shadows", true);

      return std::make_shared<Aerial_perspective_factory>(settings, num_workers, step_size,
                                                          shadows);
    } else if ("Emission" == n.name) {
      float const step_size = json::read_float(n.value, "step_size", 1.f);

      return std::make_shared<Emission_factory>(settings, num_workers, step_size);
    } else if ("Flow_vis" == n.name) {
      float const step_size = json::read_float(n.value, "step_size", 1.f);

      return std::make_shared<Flow_vis_factory>(settings, num_workers, step_size);
    } else if ("Ray_marching" == n.name) {
      float const step_size = json::read_float(n.value, "step_size", 1.f);
      float const step_probability = json::read_float(n.value, "step_probability", 0.9f);

      return std::make_shared<Ray_marching_single_factory>(settings, num_workers, step_size,
                                                           step_probability);
    } else if ("Tracking" == n.name) {
      bool const multiple_scattering = json::read_bool(n.value, "multiple_scattering", true);

      if (multiple_scattering) {
        return std::make_shared<Tracking_multi_factory>(settings, num_workers);
      } else {
        return std::make_shared<Tracking_single_factory>(settings, num_workers);
      }
    }
  }

  return nullptr;
}

void Loader::load_postprocessors(json::Value const& pp_value, resource::Manager& manager,
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

      if (take.view.camera && backplate->dimensions_2() != take.view.camera->sensor_dimensions()) {
        logging::warning("Not using backplate \"" + name +
                         "\", "
                         "because resolution doesn't match sensor resolution.");
        continue;
      }

      pipeline.add(std::make_unique<Backplate>(backplate));
    } else if ("Bloom" == n->name) {
      float const angle = json::read_float(n->value, "angle", 0.05f);
      float const alpha = json::read_float(n->value, "alpha", 0.005f);
      float const threshold = json::read_float(n->value, "threshold", 2.f);
      float const intensity = json::read_float(n->value, "intensity", 0.1f);

      pipeline.add(std::make_unique<Bloom>(angle, alpha, threshold, intensity));
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

      pipeline.add(std::make_unique<Glare>(adaption, threshold, intensity));
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

      pipeline.add(std::make_unique<Glare2>(adaption, threshold, intensity));
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

      pipeline.add(std::make_unique<Glare3>(adaption, threshold, intensity));
    }
  }
}

std::unique_ptr<rendering::postprocessor::Postprocessor> Loader::load_tonemapper(
    json::Value const& tonemapper_value) {
  using namespace rendering::postprocessor::tonemapping;

  for (auto& n : tonemapper_value.GetObject()) {
    if ("ACES" == n.name) {
      float const hdr_max = json::read_float(n.value, "hdr_max", 1.f);

      return std::make_unique<Aces>(hdr_max);
    } else if ("Generic" == n.name) {
      float const contrast = json::read_float(n.value, "contrast", 1.15f);
      float const shoulder = json::read_float(n.value, "shoulder", 0.99f);
      float const mid_in = json::read_float(n.value, "mid_in", 0.18f);
      float const mid_out = json::read_float(n.value, "mid_out", 0.18f);
      float const hdr_max = json::read_float(n.value, "hdr_max", 1.f);

      return std::make_unique<Generic>(contrast, shoulder, mid_in, mid_out, hdr_max);
    } else if ("Identity" == n.name) {
      return std::make_unique<Identity>();
    } else if ("Uncharted" == n.name) {
      float const hdr_max = json::read_float(n.value, "hdr_max", 1.f);

      return std::make_unique<Uncharted>(hdr_max);
    }
  }

  return nullptr;
}

bool Loader::peek_stereoscopic(json::Value const& parameters_value) {
  auto const export_node = parameters_value.FindMember("stereo");
  if (parameters_value.MemberEnd() == export_node) {
    return false;
  }

  return true;
}

std::vector<std::unique_ptr<exporting::Sink>> Loader::load_exporters(
    json::Value const& exporter_value, const View& view) {
  if (!view.camera) {
    return {};
  }

  auto const& camera = *view.camera;

  std::vector<std::unique_ptr<exporting::Sink>> exporters;

  for (auto& n : exporter_value.GetObject()) {
    if ("Image" == n.name) {
      std::string const format = json::read_string(n.value, "format", "PNG");

      std::unique_ptr<image::Writer> writer;

      if ("RGBE" == format) {
        writer = std::unique_ptr<image::encoding::rgbe::Writer>();
      } else {
        bool const transparent_sensor = camera.sensor().has_alpha_transparency();
        if (view.pipeline.has_alpha_transparency(transparent_sensor)) {
          writer =
              std::make_unique<image::encoding::png::Writer_alpha>(camera.sensor().dimensions());
        } else {
          writer = std::make_unique<image::encoding::png::Writer>(camera.sensor().dimensions());
        }
      }

      exporters.push_back(
          std::make_unique<exporting::Image_sequence>("output_", std::move(writer)));
    } else if ("Movie" == n.name) {
      uint32_t framerate = json::read_uint(n.value, "framerate");

      if (0 == framerate) {
        framerate = static_cast<uint32_t>(1.f / camera.frame_duration() + 0.5f);
      }

      exporters.push_back(
          std::make_unique<exporting::Ffmpeg>("output", camera.sensor().dimensions(), framerate));
    } else if ("Null" == n.name) {
      exporters.push_back(std::make_unique<exporting::Null>());
    } else if ("Statistics" == n.name) {
      exporters.push_back(std::make_unique<exporting::Statistics>());
    }
  }

  return exporters;
}

void Loader::load_settings(json::Value const& settings_value, Settings& settings) {
  for (auto& n : settings_value.GetObject()) {
    if ("ray_offset_factor" == n.name) {
      settings.ray_offset_factor = json::read_float(n.value);
    }
  }
}

void Loader::load_light_sampling(json::Value const& parent_value,
                                 rendering::integrator::Light_sampling& sampling) {
  auto const light_sampling_node = parent_value.FindMember("light_sampling");
  if (parent_value.MemberEnd() == light_sampling_node) {
    return;
  }

  for (auto& n : light_sampling_node->value.GetObject()) {
    if ("strategy" == n.name) {
      std::string strategy = json::read_string(n.value);

      if ("Single" == strategy) {
        sampling.strategy = rendering::integrator::Light_sampling::Strategy::Single;
      } else if ("All" == strategy) {
        sampling.strategy = rendering::integrator::Light_sampling::Strategy::All;
      }
    } else if ("num_samples" == n.name) {
      sampling.num_samples = json::read_uint(n.value);
    }
  }
}

}  // namespace take
