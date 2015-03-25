#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "core/rendering/renderer.hpp"
#include "core/rendering/film/film.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/image/storage/writer.hpp"
#include "core/progress/std_out.hpp"
#include <chrono>
#include <thread>
#include <algorithm>
#include <iostream>

float duration_to_seconds(std::chrono::high_resolution_clock::duration duration);

int main() {
	std::cout << "Welcome to sprout!" << std::endl;

	size_t num_workers = std::max(std::thread::hardware_concurrency(), 1u) - 1;

	std::cout << "#Threads " << num_workers << std::endl;

	std::chrono::high_resolution_clock clock;

	std::cout << "Loading..." << std::endl;

	auto loading_start = clock.now();

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
		std::cout << "Scene \"" << take->scene << "\" could not be loaded." << std::endl;
		return 1;
	}

	auto loading_duration = clock.now() - loading_start;
	std::cout << "(" << duration_to_seconds(loading_duration) << "s)" << std::endl;

	rendering::Renderer renderer(take->surface_integrator_factory, take->sampler);

	progress::Std_out progressor;

	std::cout << "Rendering..." << std::endl;

	auto rendering_start = clock.now();

	renderer.render(scene, take->context, num_workers, progressor);

	auto rendering_duration = clock.now() - rendering_start;
	std::cout << "(" << duration_to_seconds(rendering_duration) << "s)" << std::endl;

	std::cout << "Exporting..." << std::endl;

	auto exporting_start = clock.now();

	bool result = image::write("output.png", take->context.camera->film().resolve());

	if (!result) {
		std::cout << "Something went wrong" << std::endl;
	}

	auto exporting_duration = clock.now() - exporting_start;
	std::cout << "(" << duration_to_seconds(exporting_duration) << "s)" << std::endl;

	return 0;
}

float duration_to_seconds(std::chrono::high_resolution_clock::duration duration) {
	std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
	return static_cast<float>(milliseconds.count()) / 1000.f;
}
