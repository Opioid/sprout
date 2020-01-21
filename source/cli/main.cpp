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
#include "core/image/texture/texture.hpp"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/log_std_out.hpp"
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

static void log_memory_consumption(resource::Manager const& manager, scene::Loader const& loader,
                                   scene::Scene const& scene) noexcept;

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

    logging::init(new logging::Std_out);

    logging::info("Welcome to sprout (" + platform::build() + " - " + platform::revision() + ")!");

    auto const args = options::parse(argc, argv);

    logging::set_verbose(args.verbose);

    if (args.take.empty()) {
        logging::error("No take specified.");
        return 1;
    }

    uint32_t const num_workers = thread::Pool::num_threads(args.threads);

    logging::info("#Threads " + string::to_string(num_workers));

    thread::Pool threads(num_workers);

    resource::Manager resources(threads);

    file::System& filesystem = resources.filesystem();

    if (args.mounts.empty()) {
        filesystem.push_mount("../data/");
    } else {
        for (auto& m : args.mounts) {
            filesystem.push_mount(m);
        }
    }

    image::Provider image_provider;
    resources.register_provider(image_provider);

    image::texture::Provider texture_provider(args.no_textures);
    auto const&              texture_resources = resources.register_provider(texture_provider);

    scene::shape::triangle::Provider mesh_provider;
    auto const&                      shape_resources = resources.register_provider(mesh_provider);

    scene::material::Provider material_provider(args.debug_material);
    auto const&               material_resources = resources.register_provider(material_provider);

    scene::Loader scene_loader(resources, material_provider.create_fallback_material());

    procedural::mesh::init(scene_loader);
    procedural::sky::init(scene_loader, material_provider);

    scene::Scene scene(scene_loader.null_shape(), shape_resources, material_resources,
                       texture_resources);

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
            bool const is_json = string::is_json(args.take);

            auto stream = is_json ? file::Stream_ptr(new std::stringstream(args.take))
                                  : filesystem.read_stream(args.take, take_name);

            if (!stream ||
                !take::Loader::load(take, *stream, take_name, args.progressive, scene, resources)) {
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

            if (args.progressive) {
                controller::progressive(take, scene, resources, threads, max_sample_size);
            } else {
                progress::Std_out progressor;

                auto const rendering_start = std::chrono::high_resolution_clock::now();

                if (take.view.camera) {
                    rendering::Driver_finalframe driver(threads, max_sample_size, progressor);

                    driver.init(take.view, scene);

                    driver.render(take.exporters);
                } else {
                    //			baking::Driver
                    // driver(take->surface_integrator_pool,
                    //								  take->volume_integrator_pool,
                    //								  take->sampler_pool);

                    //			driver.render(scene, take->view, thread_pool,
                    //*take->exporter, progressor);
                    logging::error("No camera specified.");
                }

                logging::info("Total render time %f s", chrono::seconds_since(rendering_start));

                logging::info("Total elapsed time %f s", chrono::seconds_since(loading_start));
            }

            log_memory_consumption(resources, scene_loader, scene);
        }

        if (args.quit) {
            break;
        }

        std::cout << "Press 'q' to quit, or any other key to render again." << std::endl;

        char const key = read_key();

        if ('q' == key) {
            break;
        }

        resources.increment_generation();
    }

    return 0;
}

void log_memory_consumption(resource::Manager const& manager, scene::Loader const& loader,
                            scene::Scene const& scene) noexcept {
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

    size_t const scene_num_bytes = loader.num_bytes() + scene.num_bytes();
    logging::verbose("\tScene: " + string::print_bytes(scene_num_bytes));

    size_t const total_num_bytes = image_num_bytes + material_num_bytes + mesh_num_bytes +
                                   scene_num_bytes;
    logging::verbose("\tTotal: " + string::print_bytes(total_num_bytes));
}
