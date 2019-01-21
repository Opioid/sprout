#ifndef SU_CORE_TAKE_LOADER_HPP
#define SU_CORE_TAKE_LOADER_HPP

#include <iosfwd>

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

    static void load(Take& take, std::istream& stream, Scene& scene, resource::Manager& manager);
};

}  // namespace take

#endif
