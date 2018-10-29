#include "base/math/vector3.inl"
#include "core/scene/scene_loader.hpp"
#include "grass.hpp"

namespace procedural::mesh {

static Grass grass;

void init(scene::Loader& loader) {
    loader.register_mesh_generator("Grass", &grass);
}

}  // namespace procedural::mesh
