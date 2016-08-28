#include "controller/controller_progressive.hpp"
#include "options/options.hpp"
#include "core/file/file_system.hpp"
#include "core/logging/logging.hpp"
#include "core/baking/baking_driver.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_stdout.hpp"
#include "core/rendering/rendering_driver_finalframe.hpp"
#include "core/resource/resource_manager.inl"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture_2d_provider.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "extension/procedural/sky/sky_provider.hpp"
#include "extension/procedural/mesh/mesh.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector.inl"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

#include "core/scene/material/substitute/substitute_test.hpp"

int main(int argc, char* argv[]) {
	scene::material::substitute::testing::test();

	return 0;

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

	procedural::sky::init(scene_loader, material_provider);
	procedural::mesh::init(scene_loader);

	scene::Scene scene;

	try {
		scene_loader.load(take->scene_filename, scene);

		if (take->camera_animation && take->view.camera) {
			scene.add_animation(take->camera_animation);
			scene.create_animation_stage(take->view.camera.get(),
										 take->camera_animation.get());
		}
	} catch (const std::exception& e) {
		logging::error("Scene \"" + take->scene_filename + "\" could not be loaded: "
					   + e.what() + ".");
		return 1;
	}

	logging::info("Loading time " +
				  string::to_string(chrono::seconds_since(loading_start)) + " s");

	logging::info("Rendering...");

	if (args.progressive) {
		controller::progressive(*take, scene, resource_manager, thread_pool);
	} else {
		progress::Stdout progressor;

		auto rendering_start = std::chrono::high_resolution_clock::now();

		if (take->view.camera) {
			rendering::Driver_finalframe driver(take->surface_integrator_factory,
												take->volume_integrator_factory,
												take->sampler_factory,
												scene,
												take->view,
												thread_pool);

			driver.render(*take->exporter, progressor);
		} else {
			baking::Driver driver(take->surface_integrator_factory,
								  take->volume_integrator_factory,
								  take->sampler_factory);

			driver.render(scene, take->view, thread_pool, *take->exporter, progressor);
		}

		logging::info("Total render time " +
					  string::to_string(chrono::seconds_since(rendering_start)) + " s");

		logging::info("Total elapsed time " +
					  string::to_string(chrono::seconds_since(total_start)) + " s");
	}

	logging::release();

	return 0;
}
