#ifndef SU_CORE_TAKE_TAKE_HPP
#define SU_CORE_TAKE_TAKE_HPP

#include <string>
#include <vector>
#include "take_settings.hpp"
#include "take_view.hpp"

namespace exporting {
class Sink;
}

namespace rendering::integrator {

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

    Settings settings;

    std::string scene_filename;

    take::View view;

    Photon_settings photon_settings;

    rendering::integrator::surface::Factory* surface_integrator_factory = nullptr;
    rendering::integrator::volume::Factory*  volume_integrator_factory  = nullptr;

    sampler::Factory* sampler_factory = nullptr;

    std::vector<exporting::Sink*> exporters;
};

}  // namespace take

#endif
