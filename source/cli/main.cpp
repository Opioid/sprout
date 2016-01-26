#include "options/options.hpp"
#include "core/file/file_system.hpp"
#include "core/logging/logging.hpp"
#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "core/rendering/rendering_driver.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_stdout.hpp"
#include "base/chrono/chrono.hpp"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

int main(int argc, char* argv[]) {
	logging::init(logging::Type::Stdout);
	logging::info("Welcome to sprout!");

	options::init(argc, argv);

	auto& args = options::options();

	file::System file_system;

	if (args.mounts.empty()) {
		file_system.push_mount("../data/");
	} else {
		for (auto& m : args.mounts) {
			file_system.push_mount(m);
		}
	}

	std::chrono::high_resolution_clock clock;

	auto total_start = clock.now();

	uint32_t available_threads = static_cast<uint32_t>(std::max(std::thread::hardware_concurrency(), 1u));
	uint32_t num_workers;
	if (args.threads <= 0) {
		num_workers = std::max(available_threads - static_cast<uint32_t>(-args.threads), 1u);
	} else {
		num_workers = std::min(available_threads, static_cast<uint32_t>(std::max(args.threads, 1)));
	}

	logging::info("#Threads " + string::to_string(num_workers));

	thread::Pool thread_pool(num_workers);

	logging::info("Loading...");

	auto loading_start = clock.now();

	std::shared_ptr<take::Take> take;

	try {
		take::Loader take_loader;
		take = take_loader.load(*file_system.read_stream(args.take));
	} catch (const std::exception& e) {
		logging::error("Take \"" + args.take + "\" could not be loaded: " + e.what() + ".");
		return 1;
	}

	// The scene loader must be alive during rendering, otherwise some resources might be released prematurely.
	// This is confusing and should be adressed.
	scene::Loader scene_loader(file_system, thread_pool);
	scene::Scene scene;

	try {
		scene_loader.load(*file_system.read_stream(take->scene), scene);

		if (take->camera_animation && take->context.camera) {
			scene.add_animation(take->camera_animation);
			scene.create_animation_stage(take->context.camera.get(), take->camera_animation.get());
		}
	} catch (const std::exception& e) {
		logging::error("Scene \"" + take->scene + "\" could not be loaded: " + e.what() + ".");
		return 1;
	}

	auto loading_duration = clock.now() - loading_start;
	logging::info("Loading time " + string::to_string(chrono::duration_to_seconds(loading_duration)) + " s");

	rendering::Driver driver(take->surface_integrator_factory, take->volume_integrator_factory, take->sampler);

	progress::Stdout progressor;

	logging::info("Rendering...");

	auto rendering_start = clock.now();

	driver.render(scene, take->context, thread_pool, *take->exporter, progressor);

	auto rendering_duration = clock.now() - rendering_start;
	logging::info("Total render time " + string::to_string(chrono::duration_to_seconds(rendering_duration)) + " s");

	auto total_duration = clock.now() - total_start;
	logging::info("Total elapsed time " + string::to_string(chrono::duration_to_seconds(total_duration)) + " s");

	logging::release();

	return 0;
}
