#pragma once

#include <cstdint>

namespace take {
struct View;
}

namespace scene {
class Scene;
}

namespace sampler {
class Pool;
}

namespace thread {
class Pool;
}

namespace exporting {
class Sink;
}

namespace progress {
class Sink;
}

namespace rendering::integrator {

namespace surface {
class Pool;
}
namespace volume {
class Pool;
}

}  // namespace rendering::integrator

namespace baking {

class Driver {
  public:
    using Surface_integrator_pool = rendering::integrator::surface::Pool;
    using Volume_integrator_pool  = rendering::integrator::volume::Pool;

    Driver(Surface_integrator_pool* surface_integrator_pool,
           Volume_integrator_pool* volume_integrator_pool, sampler::Pool* sampler_pool);

    void render(scene::Scene& scene, const take::View& view, thread::Pool& thread_pool,
                uint32_t max_sample_size, exporting::Sink& exporter, progress::Sink& progressor);

  private:
    Surface_integrator_pool* surface_integrator_pool_;
    Volume_integrator_pool*  volume_integrator_pool_;
    sampler::Pool*           sampler_pool_;
};

}  // namespace baking
