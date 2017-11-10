#include "grass.hpp"
#include "core/scene/scene_loader.hpp"
#include "base/math/vector3.inl"

namespace procedural::mesh {

Grass grass;

void init(scene::Loader& loader) {
	loader.register_mesh_generator("Grass", &grass);
}

}
