#include "scene_bvh_builder_base.hpp"
#include "base/math/aabb.inl"

namespace scene::bvh {

Builder_base::Build_node::Build_node() noexcept = default;

Builder_base::Build_node::~Build_node() noexcept {
    delete children[0];
    delete children[1];
}

void Builder_base::Build_node::clear() noexcept {
    delete children[0];
    children[0] = nullptr;

    delete children[1];
    children[1] = nullptr;

    start_index = 0;
    end_index = 0;

    // This size will be used even if there are only infinite props in the scene.
    // It is an arbitrary size that will be used to calculate the power of some lights.
    aabb = AABB(float3(-1.f), float3(1.f));
}

}  // namespace scene::shape::triangle::bvh
