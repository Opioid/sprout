#ifndef SU_CORE_TAKE_LOADER_HPP
#define SU_CORE_TAKE_LOADER_HPP

#include <iosfwd>
#include <string_view>

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
    using Scene = scene::Scene;

    static void load(Take& take, std::istream& stream, std::string_view take_name, Scene& scene,
                     resource::Manager& manager);
};

}  // namespace take

#endif
