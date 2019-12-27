#ifndef SU_CORE_TAKE_LOADER_HPP
#define SU_CORE_TAKE_LOADER_HPP

#include "base/json/json_types.hpp"
#include "base/math/vector2.hpp"

#include <iosfwd>
#include <string_view>

namespace rendering::postprocessor {
class Pipeline;
}

namespace resource {
class Manager;
}

namespace scene {
namespace camera {
class Camera;
}

class Scene;
}  // namespace scene

namespace take {

struct Take;

class Loader {
  public:
    using Scene    = scene::Scene;
    using Camera   = scene::camera::Camera;
    using Pipeline = rendering::postprocessor::Pipeline;

    static bool load(Take& take, std::istream& stream, std::string_view take_name, Scene& scene,
                     resource::Manager& resources) noexcept;

    static Camera* load_camera(json::Value const& camera_value, Scene& scene) noexcept;

    static void load_integrator_factories(json::Value const& integrator_value, uint32_t num_workers,
                                          Take& take) noexcept;

    static void load_postprocessors(json::Value const& pp_value, resource::Manager& resources,
                                    Pipeline& pipeline, int2 dimensions) noexcept;
};

}  // namespace take

#endif
