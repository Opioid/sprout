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
class Scene;
}  // namespace scene

namespace take {

struct Take;

class Loader {
  public:
    using Scene    = scene::Scene;
    using Pipeline = rendering::postprocessor::Pipeline;

    static bool load(Take& take, std::istream& stream, std::string_view take_name, Scene& scene,
                     resource::Manager& manager) noexcept;

    static void load_postprocessors(json::Value const& pp_value, resource::Manager& manager,
                                    Pipeline& pipeline, int2 dimensions) noexcept;
};

}  // namespace take

#endif
