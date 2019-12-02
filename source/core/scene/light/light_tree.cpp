#include "light_tree.hpp"

namespace scene::light {

Tree::Result Tree::random_light(float3 const& p, float random) const noexcept {
    return {0.f, 0xFFFFFFFF};
}

void Tree_builder::build(Tree& tree, Scene& const scene) const noexcept {

}

}
