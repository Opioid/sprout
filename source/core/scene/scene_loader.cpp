#include "scene_loader.hpp"
#include "scene.hpp"
#include "scene/surrounding/uniform_surrounding.hpp"
#include "base/math/vector.inl"

namespace scene {

bool Loader::load(const std::string& filename, Scene& scene) {

	math::float3 energy(0.2f, 0.4f, 0.1f);
	surrounding::Uniform* uniform = new surrounding::Uniform(energy);

	scene.set_surrounding(uniform);

	return true;
}

}
