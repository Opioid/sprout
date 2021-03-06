#ifndef SU_IT_OPERATOR_DIFFERENCE_HPP
#define SU_IT_OPERATOR_DIFFERENCE_HPP

#include <cstdint>
#include <vector>

namespace scene {
class Scene;
}

using Scene = scene::Scene;

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace it::options {
struct Options;
}

struct Item;

namespace op {
uint32_t difference(std::vector<Item> const& items, it::options::Options const& options,
                    Scene const& scene, Threads& threads);
}

#endif
