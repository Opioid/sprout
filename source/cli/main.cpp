#include "controller/controller_progressive.hpp"
#include "options/options.hpp"
#include "core/baking/baking_driver.hpp"
#include "core/file/file_system.hpp"
#include "core/logging/logging.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture_provider.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_stdout.hpp"
#include "core/rendering/rendering_driver_finalframe.hpp"
#include "core/rendering/sensor/sensor.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/camera/camera.hpp"
#include "core/take/take_loader.hpp"
#include "core/take/take.hpp"
#include "extension/procedural/sky/sky_provider.hpp"
#include "extension/procedural/mesh/mesh.hpp"
#include "base/chrono/chrono.hpp"
#include "base/platform/platform.hpp"
#include "base/string/string.inl"
#include "base/thread/thread_pool.hpp"

#include "extension/procedural/starburst/starburst.hpp"
#include "core/testing/testing_simd.hpp"
#include "core/testing/testing_size.hpp"
#include "core/testing/testing_spectrum.hpp"

void log_memory_consumption(const resource::Manager& manager,
							const take::Take& take,
							size_t rendering_num_bytes);

int main(int argc, char* argv[]) {
//	testing::size();
//	testing::simd::rsqrt();
//	testing::simd::rcp();
//	testing::simd::normalize();
//	testing::simd::reciprocal();
//	testing::simd::dot();
//	testing::simd::minmax();
//	testing::simd::unions();
//	testing::spectrum();
//	return 1;

	logging::init(logging::Type::Stdout);
	logging::info("Welcome to sprout (" + platform::build() +  ")!");

	options::init(argc, argv);

	const auto& args = options::options();

	file::System file_system;

	if (args.mounts.empty()) {
		file_system.push_mount("../data/");
	} else {
		for (auto& m : args.mounts) {
			file_system.push_mount(m);
		}
	}

	const auto total_start = std::chrono::high_resolution_clock::now();

	const uint32_t available_threads = std::max(std::thread::hardware_concurrency(), 1u);
	uint32_t num_workers;
	if (args.threads <= 0) {
		num_workers = static_cast<uint32_t>(
					std::max(static_cast<int32_t>(available_threads) + args.threads, 1));
	} else {
		num_workers = std::min(available_threads, static_cast<uint32_t>(std::max(args.threads, 1)));
	}

	logging::info("#Threads " + string::to_string(num_workers));

	thread::Pool thread_pool(num_workers);




//	auto starburst_start = std::chrono::high_resolution_clock::now();
//	procedural::starburst::create(thread_pool);
//	logging::info("Starburst time " +
//				  string::to_string(chrono::seconds_since(starburst_start)) + " s");

//	return 0;







	logging::info("Loading...");

	const auto loading_start = std::chrono::high_resolution_clock::now();

	std::shared_ptr<take::Take> take;

	try {
		take = take::Loader::load(*file_system.read_stream(args.take), thread_pool);
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
	if (!args.no_textures) {
		resource_manager.register_provider(texture_provider);
	}

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
		logging::release();
		return 1;
	}

	logging::info("Loading time " +
				  string::to_string(chrono::seconds_since(loading_start)) + " s");

	logging::info("Rendering...");

	size_t rendering_num_bytes = 0;

	if (args.progressive) {
		rendering_num_bytes = controller::progressive(*take, scene, resource_manager, thread_pool);
	} else {
		progress::Stdout progressor;

		const auto rendering_start = std::chrono::high_resolution_clock::now();

		if (take->view.camera) {
			rendering::Driver_finalframe driver(take->surface_integrator_factory,
												take->volume_integrator_factory,
												take->sampler_factory,
												scene,
												take->view,
												thread_pool);

			rendering_num_bytes += driver.num_bytes();

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

	log_memory_consumption(resource_manager, *take, rendering_num_bytes);

	logging::release();

	return 0;
}

void log_memory_consumption(const resource::Manager& manager,
							const take::Take& take,
							size_t rendering_num_bytes) {
	logging::info("Memory consumption:");

	size_t image_num_bytes = manager.num_bytes<image::Image>();
	logging::info("Images: " + string::print_bytes(image_num_bytes));

	size_t material_num_bytes = manager.num_bytes<scene::material::Material>();
	logging::info("Materials: " + string::print_bytes(material_num_bytes));

	size_t mesh_num_bytes = manager.num_bytes<scene::shape::Shape>();
	logging::info("Meshes: " + string::print_bytes(mesh_num_bytes));

	size_t renderer_num_bytes = take.view.pipeline.num_bytes()
							  + take.view.camera->sensor().num_bytes()
							  + rendering_num_bytes;
	logging::info("Renderer: " + string::print_bytes(renderer_num_bytes));

	size_t total_num_bytes = image_num_bytes + material_num_bytes
						   + mesh_num_bytes + renderer_num_bytes;
	logging::info("Total: " + string::print_bytes(total_num_bytes));
}
