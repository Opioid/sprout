#ifndef SU_CORE_SCENE_BVH_BUILDER_HPP
#define SU_CORE_SCENE_BVH_BUILDER_HPP

#include "base/math/aabb.hpp"
#include "scene_bvh_builder_base.hpp"

#include <cstddef>
#include <vector>

namespace thread {
class Pool;
}

namespace scene::bvh {

class Node;
struct Tree;

class Builder final : private Builder_base {
  public:
    Builder();

    ~Builder() final;

    void build(Tree& tree, std::vector<uint32_t>& indices, std::vector<AABB> const& aabbs,
               thread::Pool& threads);

  private:
    void serialize(Tree& tree) const;
};

}  // namespace scene::bvh

#endif
