#include "grass.hpp"
#include "core/scene/scene_loader.hpp"
#include "base/math/vector.inl"

namespace procedural { namespace mesh {

Grass grass;

void init(scene::Loader& loader) {
	loader.register_mesh_generator("Grass", &grass);
}

}}

