#include "any_key.hpp"
#include "base/chrono/chrono.hpp"
#include "base/platform/platform.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "controller/controller_progressive.hpp"
#include "core/baking/baking_driver.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/logging.hpp"
#include "core/progress/progress_sink_null.hpp"
#include "core/progress/progress_sink_std_out.hpp"
#include "core/rendering/rendering_driver_finalframe.hpp"
#include "core/rendering/sensor/sensor.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/camera/camera.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/shape/shape.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/take/take.hpp"
#include "core/take/take_loader.hpp"
#include "extension/procedural/mesh/mesh.hpp"
#include "extension/procedural/sky/sky_provider.hpp"
#include "options/options.hpp"

#include <iostream>
#include <istream>
#include <sstream>

//#include "core/scene/material/substitute/substitute_test.hpp"
//#include "core/scene/material/glass/glass_test.hpp"
//#include "core/testing/testing_cdf.hpp"
//#include "core/testing/testing_simd.hpp"
//#include "core/testing/testing_size.hpp"
//#include "core/testing/testing_spectrum.hpp"
//#include "core/testing/testing_vector.hpp"
//#include "core/sampler/sampler_test.hpp"
//#include "core/scene/material/ggx/ggx_integrate.hpp"

static void log_memory_consumption(resource::Manager const& manager, take::Take const& take,
                                   scene::Loader const& loader, scene::Scene const& scene,
                                   size_t rendering_num_bytes) noexcept;

static bool is_json(std::string const& text) noexcept;

int main(int argc, char* argv[]) noexcept {
    //	scene::material::substitute::testing::test();
    //	scene::material::glass::testing::test();
    //  scene::material::glass::testing::rough_refraction();
    //  testing::size();
    //  testing::simd::rsqrt();
    //	testing::simd::rcp();
    //  testing::simd::normalize();
    //	testing::simd::reciprocal();
    //	testing::simd::dot();
    //	testing::simd::minmax();
    //	testing::simd::unions();
    //	testing::simd::basis();
    //	testing::spectrum();
    //  testing::vector();
    //	testing::cdf::test_1D();
    //  sampler::testing::test();

    //  scene::material::ggx::integrate();

    //  return 1;

    logging::init(logging::Type::Std_out);

    logging::info("Welcome to sprout (" + platform::build() + " - " + platform::revision() + ")!");

    auto const args = options::parse(argc, argv);

    logging::set_verbose(args.verbose);

    if (args.take.empty()) {
        logging::error("No take specified.");
        return 1;
    }

    file::System file_system;

    if (args.mounts.empty()) {
        file_system.push_mount("../data/");
    } else {
        for (auto& m : args.mounts) {
            file_system.push_mount(m);
        }
    }

    uint32_t const available_threads = std::max(std::thread::hardware_concurrency(), 1u);

    uint32_t num_workers;
    if (args.threads <= 0) {
        int32_t const num_threads = static_cast<int32_t>(available_threads) + args.threads;

        num_workers = uint32_t(std::max(num_threads, 1));
    } else {
        num_workers = std::min(available_threads, uint32_t(std::max(args.threads, 1)));
    }

    logging::info("#Threads " + string::to_string(num_workers));

    thread::Pool thread_pool(num_workers);

    resource::Manager resource_manager(file_system, thread_pool);

    image::Provider image_provider;
    resource_manager.register_provider(image_provider);

    image::texture::Provider texture_provider;
    if (!args.no_textures) {
        resource_manager.register_provider(texture_provider);
    }

    scene::shape::triangle::Provider mesh_provider;
    auto const& shape_resources = resource_manager.register_provider(mesh_provider);

    scene::material::Provider material_provider(args.debug_material);
    auto const& material_resources = resource_manager.register_provider(material_provider);

    scene::Loader scene_loader(resource_manager, material_provider.create_fallback_material());

    procedural::mesh::init(scene_loader);
    procedural::sky::init(scene_loader, material_provider);

    scene::Scene scene(scene_loader.null_shape(), shape_resources, material_resources);

    std::string take_name;

    take::Take take;

    uint32_t const max_sample_size = material_provider.max_sample_size();

    for (;;) {
        logging::info("Loading...");

        auto const loading_start = std::chrono::high_resolution_clock::now();

        take.clear();
        scene.clear();

        bool success = true;

        {
            auto stream = is_json(args.take) ? file::Stream_ptr(new std::stringstream(args.take))
                                             : file_system.read_stream(args.take, take_name);

            if (!stream || !take::Loader::load(take, *stream, take_name, scene, resource_manager)) {
                logging::error("Loading take %S: ", args.take);
                success = false;
            }
        }

        if (success && !scene_loader.load(take.scene_filename, take_name, take, scene)) {
            logging::error("Loading scene %S: ", take.scene_filename);
            success = false;
        }

        if (success) {
            logging::info("Loading time %f s", chrono::seconds_since(loading_start));

            logging::info("Rendering...");

            size_t rendering_num_bytes = 0;

            if (args.progressive) {
                rendering_num_bytes = controller::progressive(take, scene, resource_manager,
                                                              thread_pool, max_sample_size);
            } else {
                progress::Std_out progressor;

                auto const rendering_start = std::chrono::high_resolution_clock::now();

                if (take.view.camera) {
                    rendering::Driver_finalframe driver(take, scene, thread_pool, max_sample_size,
                                                        progressor);

                    rendering_num_bytes += driver.num_bytes();

                    driver.render(take.exporters);
                } else {
                    //			baking::Driver
                    // driver(take->surface_integrator_factory,
                    //								  take->volume_integrator_factory,
                    //								  take->sampler_factory);

                    //			driver.render(scene, take->view, thread_pool,
                    //*take->exporter, progressor);
                    logging::error("No camera specified.");
                }

                logging::info("Total render time %f s", chrono::seconds_since(rendering_start));

                logging::info("Total elapsed time %f s", chrono::seconds_since(loading_start));
            }

            log_memory_consumption(resource_manager, take, scene_loader, scene,
                                   rendering_num_bytes);
        }

        if (args.quit) {
            break;
        }

        std::cout << "Press 'q' to quit, or any other key to render again." << std::endl;

        char const key = read_key();

        if ('q' == key) {
            break;
        }

        resource_manager.increment_generation();
    }

    return 0;
}

void log_memory_consumption(resource::Manager const& manager, take::Take const& take,
                            scene::Loader const& loader, scene::Scene const& scene,
                            size_t rendering_num_bytes) noexcept {
    if (!logging::is_verbose()) {
        return;
    }

    logging::verbose("Memory consumption:");

    size_t const image_num_bytes = manager.num_bytes<image::Image>();
    logging::verbose("\tImages: " + string::print_bytes(image_num_bytes));

    size_t const material_num_bytes = manager.num_bytes<scene::material::Material>();
    logging::verbose("\tMaterials: " + string::print_bytes(material_num_bytes));

    size_t const mesh_num_bytes = manager.num_bytes<scene::shape::Shape>();
    logging::verbose("\tMeshes: " + string::print_bytes(mesh_num_bytes));

    size_t const renderer_num_bytes = take.view.pipeline.num_bytes() +
                                      take.view.camera->sensor().num_bytes() + rendering_num_bytes;
    logging::verbose("\tRenderer: " + string::print_bytes(renderer_num_bytes));

    size_t const scene_num_bytes = loader.num_bytes() + scene.num_bytes();
    logging::verbose("\tScene: " + string::print_bytes(scene_num_bytes));

    size_t const total_num_bytes = image_num_bytes + material_num_bytes + mesh_num_bytes +
                                   renderer_num_bytes + scene_num_bytes;
    logging::verbose("\tTotal: " + string::print_bytes(total_num_bytes));
}

bool is_json(std::string const& text) noexcept {
    auto const it = text.find_first_not_of(" \t");

    if (std::string::npos != it) {
        return '{' == text[it];
    }

    return false;
}
