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
#include "core/scene/camera/camera.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/prop/prop.hpp"
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

using namespace scene;

static bool load_take_and_scene(std::string const& take_string, scene::Loader& scene_loader,
                                resource::Manager& resources, uint32_t frame, take::Take& take,
                                Scene& scene);

static bool reload_frame_dependant(scene::Loader& scene_loader, resource::Manager& resources,
                                   uint32_t frame, take::Take& take, Scene& scene);

int main(int argc, char* argv[]) {
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
    auto const&     image_resources = resources.register_provider(image_provider);

    shape::triangle::Provider mesh_provider;
    auto const&               shape_resources = resources.register_provider(mesh_provider);

    material::Provider material_provider(args.no_tex_dwim, args.debug_material);
    auto const&        material_resources = resources.register_provider(material_provider);

    scene::Loader scene_loader(resources, material_provider.create_fallback_material());

    procedural::mesh::init(scene_loader);
    procedural::sky::init(scene_loader, material_provider);

    Scene scene(image_resources, material_resources, shape_resources, scene_loader.null_shape());

    std::string take_name;

    take::Take take;

    SOFT_ASSERT(material::Sample_cache::Max_sample_size >= material::Provider::max_sample_size());

    progress::Std_out progressor;
    rendering::Driver driver(threads, progressor);

    for (;;) {
        take.clear();
        scene.clear();

        logging::info("Loading...");

        auto const loading_start = std::chrono::high_resolution_clock::now();

        if (load_take_and_scene(args.take, scene_loader, resources, args.start_frame, take,
                                scene)) {
            logging::info("Loading time %f s", chrono::seconds_since(loading_start));
            logging::info("Rendering...");

            auto const rendering_start = std::chrono::high_resolution_clock::now();

            driver.init(take.view, scene, false);

            for (uint32_t f = args.start_frame, end = args.start_frame + args.num_frames; f < end;
                 ++f) {
                if (!reload_frame_dependant(scene_loader, resources, f, take, scene)) {
                    continue;
                }

                driver.render(f);
                driver.export_frame(f, take.exporters);
            }

            logging::info("Total render time %f s", chrono::seconds_since(rendering_start));
            logging::info("Total elapsed time %f s", chrono::seconds_since(loading_start));
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
        image_provider.increment_generation();

        if (args.num_frames > 1) {
            resources.deprecate_frame_dependant<image::Image>();
        }
    }

    return 0;
}

static bool load_take_and_scene(std::string const& take_string, scene::Loader& scene_loader,
                                resource::Manager& resources, uint32_t frame, take::Take& take,
                                Scene& scene) {
    file::System& filesystem = resources.filesystem();

    filesystem.set_frame(frame);

    bool const is_json = string::is_json(take_string);

    auto stream = is_json ? filesystem.string_stream(take_string)
                          : filesystem.read_stream(take_string, take.resolved_name);

    if (!stream || !take::Loader::load(take, *stream, false, scene, resources)) {
        logging::error("Loading take %S: ", take_string);
        return false;
    }

    if (!scene_loader.load(take.scene_filename, take, scene)) {
        logging::error("Loading scene %S: ", take.scene_filename);
        return false;
    }

    return true;
}

static bool reload_frame_dependant(scene::Loader& scene_loader, resource::Manager& resources,
                                   uint32_t frame, take::Take& take, Scene& scene) {
    file::System& filesystem = resources.filesystem();

    if (frame == filesystem.frame()) {
        return true;
    }

    filesystem.set_frame(frame);

    if (!resources.reload_frame_dependant<image::Image>()) {
        return true;
    }

    logging::info("Loading...");

    auto const loading_start = std::chrono::high_resolution_clock::now();

    scene.commit_materials(resources.threads());

    take.view.camera->set_entity(prop::Null);

    scene.clear();

    if (!scene_loader.load(take.scene_filename, take, scene)) {
        logging::error("Loading scene %S: ", take.scene_filename);
        return false;
    }

    logging::info("Loading time %f s", chrono::seconds_since(loading_start));

    return true;
}
