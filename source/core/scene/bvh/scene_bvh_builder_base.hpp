#ifndef SU_CORE_SCENE_BVH_BUILDER_BASE_HPP
#define SU_CORE_SCENE_BVH_BUILDER_BASE_HPP

#include "base/math/aabb.hpp"

#include <cstdint>
#include <vector>

namespace scene::bvh {

class Builder_base {
  protected:
    struct Build_node {
        Build_node() noexcept;

        ~Build_node() noexcept;

        void clear() noexcept;

        AABB aabb;

        uint8_t axis;


        std::vector<uint32_t> primitives;

        uint32_t start_index = 0;
        uint32_t end_index = 0;



        Build_node* children[2] = {nullptr, nullptr};
    };
};

}  // namespace bvh

#endif

