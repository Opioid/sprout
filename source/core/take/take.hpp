#ifndef SU_CORE_TAKE_TAKE_HPP
#define SU_CORE_TAKE_TAKE_HPP

#include "base/memory/array.hpp"
#include "take_view.hpp"

#include <string>

namespace exporting {
class Sink;
}

namespace rendering::integrator {

namespace particle {
class Lighttracer_factory;
}
namespace surface {
class Factory;
}
namespace volume {
class Factory;
}

}  // namespace rendering::integrator

namespace sampler {
class Factory;
}

namespace take {

struct Take {
    Take() noexcept;

    ~Take() noexcept;

    void clear() noexcept;

    std::string scene_filename;

    View view;

    rendering::integrator::surface::Factory*              surface_integrator_factory = nullptr;
    rendering::integrator::volume::Factory*               volume_integrator_factory  = nullptr;
    rendering::integrator::particle::Lighttracer_factory* lighttracer_factory        = nullptr;

    sampler::Factory* sampler_factory = nullptr;

    memory::Array<exporting::Sink*> exporters;
};

}  // namespace take

#endif
