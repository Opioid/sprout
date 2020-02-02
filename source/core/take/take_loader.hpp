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
struct View;

class Loader {
  public:
    using Scene     = scene::Scene;
    using Camera    = scene::camera::Camera;
    using Pipeline  = rendering::postprocessor::Pipeline;
    using Resources = resource::Manager;

    static bool load(Take& take, std::istream& stream, std::string_view take_name, bool progressive,
                     Scene& scene, Resources& resources);

    static Camera* load_camera(json::Value const& camera_value, Scene& scene);

    static void load_integrators(json::Value const& integrator_value, uint32_t num_workers,
                                 bool progressive, View& view);

    static void set_default_integrators(uint32_t num_workers, bool progressive, View& view);

    static void load_postprocessors(json::Value const& pp_value, Resources& resources,
                                    Pipeline& pipeline, int2 dimensions);

    static void set_default_exporter(Take& take);
};

}  // namespace take

#endif
