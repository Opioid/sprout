#include "options/options.hpp"
#include "core/file/file_system.hpp"
#include "core/logging/logging.hpp"
#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "core/rendering/renderer.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_stdout.hpp"
#include "base/chrono/chrono.hpp"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

int main(int argc, char* argv[]) {
	logging::init(logging::Type::Stdout);

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

	logging::info("Welcome to sprout!");

	uint32_t available_threads = static_cast<uint32_t>(std::max(std::thread::hardware_concurrency(), 1u));
	uint32_t num_workers;
	if (args.threads <= 0) {
		num_workers = std::max(available_threads - static_cast<uint32_t>(-args.threads), 1u);
	} else {
		num_workers = std::min(available_threads, static_cast<uint32_t>(std::max(args.threads, 1)));
	}

//	uint32_t num_workers = /*1;//*/static_cast<uint32_t>(std::max(std::thread::hardware_concurrency(), 1u)) - 1;

	logging::info("#Threads " + string::to_string(num_workers));

	thread::Pool pool(num_workers);

	logging::info("Loading...");

	auto loading_start = clock.now();

	std::string takename = args.take;
	if (takename.empty()) {
		takename = "takes/sponza.take";
	}

	std::shared_ptr<take::Take> take;

	try {
		take::Loader take_loader;
		take = take_loader.load(*file_system.read_stream(takename));
	} catch (const std::exception& e) {
		logging::error("Take \"" + takename + "\" could not be loaded: " + e.what() + ".");
		return 1;
	}

	// The scene loader must be alive during rendering, otherwise some resources might be released prematurely.
	// This is confusing and should be adressed.
	scene::Loader scene_loader(file_system, num_workers, pool);
	scene::Scene scene;

	try {
		scene_loader.load(*file_system.read_stream(take->scene), scene);
	} catch (const std::exception& e) {
		logging::error("Scene \"" + take->scene + "\" could not be loaded: " + e.what() + ".");
		return 1;
	}

	auto loading_duration = clock.now() - loading_start;
	logging::info("Loading time " + string::to_string(chrono::duration_to_seconds(loading_duration)) + " s");

	rendering::Renderer renderer(take->surface_integrator_factory, take->sampler);

	progress::Stdout progressor;

	logging::info("Rendering...");

	auto rendering_start = clock.now();

	renderer.render(scene, take->context, pool, *take->exporter, progressor);

	auto rendering_duration = clock.now() - rendering_start;
	logging::info("Total render time " + string::to_string(chrono::duration_to_seconds(rendering_duration)) + " s");

	auto total_duration = clock.now() - total_start;
	logging::info("Total elapsed time " + string::to_string(chrono::duration_to_seconds(total_duration)) + " s");

	logging::release();

	return 0;
}
