#ifndef SU_CORE_TAKE_TAKE_HPP
#define SU_CORE_TAKE_TAKE_HPP

#include "base/memory/array.hpp"
#include "rendering/postprocessor/postprocessor_pipeline.hpp"

#include <string>

namespace scene::camera {
class Camera;
}

namespace exporting {
class Sink;
}

namespace rendering::integrator {

namespace particle {
class Lighttracer_pool;
}
namespace surface {
class Pool;
}
namespace volume {
class Pool;
}

}  // namespace rendering::integrator

namespace sampler {
class Pool;
}

namespace take {

struct Photon_settings {
    uint32_t num_photons = 0;
    uint32_t max_bounces = 2;

    float iteration_threshold  = 0.f;
    float search_radius        = 0.01f;
    float merge_radius         = 0.0025f;
    float coarse_search_radius = 0.1f;

    bool full_light_path = false;
};

struct View {
    View();

    ~View();

    void clear();

    void init(thread::Pool& threads);

    bool valid() const;

    rendering::integrator::surface::Pool* surface_integrators = nullptr;

    rendering::integrator::volume::Pool* volume_integrators = nullptr;

    rendering::integrator::particle::Lighttracer_pool* lighttracers = nullptr;

    sampler::Pool* samplers = nullptr;

    scene::camera::Camera* camera = nullptr;

    rendering::postprocessor::Pipeline pipeline;

    uint32_t num_samples_per_pixel   = 1;
    uint32_t num_particles_per_pixel = 0;

    uint32_t start_frame = 0;
    uint32_t num_frames  = 1;

    Photon_settings photon_settings;
};

struct Take {
    Take();

    ~Take();

    void clear();

    std::string scene_filename;

    View view;

    memory::Array<exporting::Sink*> exporters;
};

}  // namespace take

#endif
