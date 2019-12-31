#ifndef SU_CLI_CONTROLLER_PROGRESSIVE_HPP
#define SU_CLI_CONTROLLER_PROGRESSIVE_HPP

#include <cstddef>
#include <cstdint>

namespace resource {
class Manager;
}

namespace scene {
class Scene;
}

namespace take {
struct Take;
}

namespace thread {
class Pool;
}

namespace controller {

void progressive(take::Take& take, scene::Scene& scene, resource::Manager& resource_manager,
                 thread::Pool& threads, uint32_t max_sample_size) noexcept;
}

#endif
