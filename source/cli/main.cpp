#include "options/options.hpp"
#include "core/file/file_system.hpp"
#include "core/logging/logging.hpp"
#include "core/baking/baking_driver.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_stdout.hpp"
#include "core/rendering/rendering_driver.hpp"
#include "core/resource/resource_manager.inl"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture_2d_provider.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/material/material_sample_cache.inl"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "extension/procedural/mesh/mesh.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector.inl"
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

	auto total_start = std::chrono::high_resolution_clock::now();

	uint32_t available_threads = std::max(std::thread::hardware_concurrency(), 1u);
	uint32_t num_workers;
	if (args.threads <= 0) {
		num_workers = std::max(available_threads - static_cast<uint32_t>(-args.threads), 1u);
	} else {
		num_workers = std::min(available_threads, static_cast<uint32_t>(std::max(args.threads, 1)));
	}

	logging::info("#Threads " + string::to_string(num_workers));

	thread::Pool thread_pool(num_workers);

	logging::info("Loading...");

	auto loading_start = std::chrono::high_resolution_clock::now();

	std::shared_ptr<take::Take> take;

	try {
		take::Loader take_loader;
		take = take_loader.load(*file_system.read_stream(args.take));
	} catch (const std::exception& e) {
		logging::error("Take \"" + args.take + "\" could not be loaded: " + e.what() + ".");
		return 1;
	}

	resource::Manager resource_manager(file_system, thread_pool);

	scene::shape::triangle::Provider mesh_provider;
	resource_manager.register_provider(mesh_provider);

	image::Provider image_provider;
	resource_manager.register_provider(image_provider);

	image::texture::Provider texture_provider;
	resource_manager.register_provider(texture_provider);

	scene::material::Provider material_provider(thread_pool.num_threads());
	resource_manager.register_provider(material_provider);

	// The scene loader must be alive during rendering,
	// otherwise some resources might be released prematurely.
	// This is confusing and should be adressed.
	scene::Loader scene_loader(resource_manager, material_provider.fallback_material());

	procedural::mesh::init(scene_loader);

	scene::Scene scene;

	try {
		scene_loader.load(*file_system.read_stream(take->scene), scene);

		if (take->camera_animation && take->view.camera) {
			scene.add_animation(take->camera_animation);
			scene.create_animation_stage(take->view.camera.get(), take->camera_animation.get());
		}
	} catch (const std::exception& e) {
		logging::error("Scene \"" + take->scene + "\" could not be loaded: " + e.what() + ".");
		return 1;
	}

	auto loading_duration = std::chrono::high_resolution_clock::now() - loading_start;
	logging::info("Loading time " +
				  string::to_string(chrono::duration_to_seconds(loading_duration)) + " s");

	progress::Stdout progressor;

	logging::info("Rendering...");

	auto rendering_start = std::chrono::high_resolution_clock::now();

	if (take->view.camera) {
		rendering::Driver driver(take->surface_integrator_factory,
								 take->volume_integrator_factory, take->sampler);

		driver.render(scene, take->view, thread_pool, *take->exporter, progressor);
	} else {
		baking::Driver driver(take->surface_integrator_factory,
							  take->volume_integrator_factory, take->sampler);

		driver.render(scene, take->view, thread_pool, *take->exporter, progressor);
	}

	auto rendering_duration = std::chrono::high_resolution_clock::now() - rendering_start;
	logging::info("Total render time " +
				  string::to_string(chrono::duration_to_seconds(rendering_duration)) + " s");

	auto total_duration = std::chrono::high_resolution_clock::now() - total_start;
	logging::info("Total elapsed time " +
				  string::to_string(chrono::duration_to_seconds(total_duration)) + " s");

	logging::release();

	return 0;
}
