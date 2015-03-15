#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "core/rendering/renderer.hpp"
#include "core/rendering/film/film.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/image/storage/writer.hpp"
#include "base/math/vector.inl"
#include <iostream>

int main() {
	std::cout << "Welcome to sprout!" << std::endl;

	std::string takename = "../data/takes/imrod.take";
	take::Loader take_loader;
	auto take = take_loader.load(takename);
	if (!take) {
		std::cout << "Take \"" << takename << "\" could not be loaded." << std::endl;
		return 1;
	}

	scene::Scene scene;
	scene::Loader scene_loader;

	if (!scene_loader.load(take->scene, scene)) {

		return 1;
	}

	rendering::Renderer renderer(take->surface_integrator_factory);

	renderer.render(scene, take->context);

	bool result = image::write("output.png", take->context.camera->film().resolve());

	if (result) {
		std::cout << "We wrote output.png" << std::endl;
	} else {
		std::cout << "Something went wrong" << std::endl;
	}

	return 0;
}
