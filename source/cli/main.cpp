#include "any_key.hpp"
#include "base/chrono/chrono.hpp"
#include "base/debug/assert.hpp"
#include "base/platform/platform.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.hpp"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/log_std_out.hpp"
#include "core/logging/logging.hpp"
#include "core/progress/progress_sink_std_out.hpp"
#include "core/rendering/rendering_driver.hpp"
#include "core/resource/resource_manager.inl"
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

#ifdef SU_DEBUG
#include "core/scene/material/material_sample_cache.hpp"
#endif

//#include "core/scene/material/substitute/substitute_test.hpp"
//#include "core/scene/material/glass/glass_test.hpp"
//#include "core/testing/testing_cdf.hpp"
//#include "core/testing/testing_simd.hpp"
//#include "core/testing/testing_size.hpp"
//#include "core/testing/testing_spectrum.hpp"
//#include "core/testing/testing_vector.hpp"
//#include "core/sampler/sampler_test.hpp"
//#include "core/scene/material/ggx/ggx_integrate.hpp"
//#include "core/image/texture/texture_encoding.hpp"

int main(int argc, char* argv[]) {
    using namespace scene;

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
    //  image::texture::encoding::create_tables();

    //  return 1;

    logging::init(new logging::Std_out);

    logging::info("Welcome to sprout (" + platform::build() + " - " + platform::revision() + ")!");

    auto const args = options::parse(argc, argv);

    if (args.take.empty()) {
        logging::error("No take specified.");
        return 1;
    }

    uint32_t const num_workers = Threads::num_threads(args.threads);

    logging::info("#Threads " + string::to_string(num_workers));

    Threads threads(num_workers);

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

    shape::triangle::Provider mesh_provider;
    auto const&               shape_resources = resources.register_provider(mesh_provider);

    material::Provider material_provider(args.no_tex_dwim, args.debug_material);
    auto const&        material_resources = resources.register_provider(material_provider);

    scene::Loader scene_loader(resources, material_provider.create_fallback_material());

    procedural::mesh::init(scene_loader);
    procedural::sky::init(scene_loader, material_provider);

    Scene scene(scene_loader.null_shape(), shape_resources, material_resources, texture_resources);

    std::string take_name;

    take::Take take;

    bool reload_take = true;
    bool reload_scene = true;
    bool reload_all = true;

    SOFT_ASSERT(material::Sample_cache::Max_sample_size >= material::Provider::max_sample_size());

    progress::Std_out progressor;
    rendering::Driver driver(threads, progressor);

    for (uint32_t f = args.start_frame, end = args.start_frame + args.num_frames; f < end; ++f) {
        logging::info("Loading...");

        auto const loading_start = std::chrono::high_resolution_clock::now();

        bool success = true;

        if (reload_all) {
            take.clear();
            scene.clear();
        }

        if (reload_take) {
            take.clear();

            bool const is_json = string::is_json(args.take);

            auto stream = is_json ? filesystem.string_stream(args.take)
                                  : filesystem.read_stream(args.take, take_name);

            if (!stream || !take::Loader::load(take, *stream, take_name, f, false, reload_all,
                                               scene, resources)) {
                logging::error("Loading take %S: ", args.take);
                success = false;
            }

            reload_take = false;
            reload_all = false;
        }

        if (reload_scene && success) {
            scene.clear();

            if (!scene_loader.load(take.scene_filename, take_name, take, scene)) {
            logging::error("Loading scene %S: ", take.scene_filename);
            success = false;
            }

            reload_scene = args.reload;
        }

        if (success) {
            logging::info("Loading time %f s", chrono::seconds_since(loading_start));
            logging::info("Rendering...");

            auto const rendering_start = std::chrono::high_resolution_clock::now();

            driver.init(take.view, scene, false);
            driver.render(f);
            driver.export_frame(f, take.exporters);

            logging::info("Total render time %f s", chrono::seconds_since(rendering_start));
            logging::info("Total elapsed time %f s", chrono::seconds_since(loading_start));
        }

        if (!success || f == end - 1) {
            if (args.quit) {
                break;
            }

            std::cout << "Press 'q' to quit, or any other key to render again." << std::endl;

            char const key = read_key();

            if ('q' == key) {
                break;
            }

            reload_take = true;
            reload_scene = true;
            reload_all = true;
        }

        resources.increment_generation();
        image_provider.increment_generation();
    }

    return 0;
}
