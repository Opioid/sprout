#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "core/rendering/renderer.hpp"
#include "core/rendering/film/film.hpp"
#include "core/scene/scene.hpp"
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

	std::cout << "We want to render \"" << take->scene << "\"!" << std::endl;

	scene::Scene scene;

	rendering::Renderer renderer;

	renderer.render(scene, take->context);

	bool result = image::write("output.png", take->context.camera->film().resolve());

	if (result) {
		std::cout << "We wrote output.png" << std::endl;
	} else {
		std::cout << "Something went wrong" << std::endl;
	}

	return 0;
}
