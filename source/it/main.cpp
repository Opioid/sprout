#include "base/chrono/chrono.hpp"
#include "base/json/json.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/log_std_out.hpp"
#include "core/logging/logging.hpp"
#include "core/resource/resource_manager.inl"
#include "core/scene/scene.hpp"
#include "core/take/take_loader.hpp"
#include "item.hpp"
#include "operator/add.hpp"
#include "operator/average.hpp"
#include "operator/concatenate.hpp"
#include "operator/difference.hpp"
#include "operator/statistics.hpp"
#include "options/options.hpp"

namespace scene {
namespace material {
class Material;
}
namespace shape {
class Shape;
}
}  // namespace scene

using namespace it::options;
using namespace scene;

using Pipeline  = op::Pipeline;
using Resources = resource::Manager;

void load_pipeline(std::istream& stream, std::string_view take_name, Pipeline& pipeline,
                   Resources& resources);

int main(int argc, char* argv[]) {
    auto const total_start = std::chrono::high_resolution_clock::now();

    logging::init(new logging::Std_out);

    //    logging::info("Welcome to di (" + platform::build() + " - " + platform::revision() +
    //    ")!");

    auto const args = it::options::parse(argc, argv);

    if (args.images.empty()) {
        logging::error("No images specified.");
        return 1;
    }

    uint32_t const available_threads = std::max(std::thread::hardware_concurrency(), 1u);

    uint32_t num_workers;
    if (args.threads <= 0) {
        int32_t const num_threads = int32_t(available_threads) + args.threads;

        num_workers = uint32_t(std::max(num_threads, 1));
    } else {
        num_workers = std::min(available_threads, uint32_t(std::max(args.threads, 1)));
    }

    //   logging::info("#Threads " + string::to_string(num_workers));

    Threads threads(num_workers);

    resource::Manager resources(threads);

    file::System& filesystem = resources.filesystem();

    image::Provider image_provider;
    auto const&     image_resources = resources.register_provider(image_provider);

    std::vector<scene::material::Material*> material_resources;
    std::vector<scene::shape::Shape*>       shape_resources;

    Scene scene(image_resources, material_resources, shape_resources, 0xFFFFFFFF);

    Pipeline pipeline;

    if (!args.take.empty()) {
        std::string take_name;

        bool const is_json = string::is_json(args.take);

        auto stream = is_json ? filesystem.string_stream(args.take)
                              : filesystem.read_stream(args.take, take_name);

        if (stream) {
            load_pipeline(*stream, take_name, pipeline, resources);
        } else {
            logging::error("Loading take %S: ", args.take);
        }
    }

    std::vector<Item> items;
    items.reserve(args.images.size());

    memory::Variant_map options;
    options.set("usage", texture::Provider::Usage::Color_with_alpha);

    uint32_t slot = Options::Operator::Diff == args.op ? 0xFFFFFFFF : 0;
    for (auto& i : args.images) {
        if (Texture const image = texture::Provider::load(i, options, float2(1.f), resources);
            image.is_valid()) {
            std::string const name_out = slot < args.outputs.size() ? args.outputs[slot] : "";

            items.emplace_back(Item{i, name_out, image});
        }

        ++slot;
    }

    if (items.empty()) {
        return 1;
    }

    if (Options::Operator::Undefined == args.op || !args.statistics.empty()) {
        op::statistics(items, args, scene, resources.threads());
    }

    if (Options::Operator::Add == args.op) {
        if (uint32_t const num = op::add(items, args, scene, resources.threads()); num) {
            logging::info("add " + string::to_string(num) + " images in " +
                          string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    } else if (Options::Operator::Average == args.op) {
        if (uint32_t const num = op::average(items, args, scene, resources.threads()); num) {
            logging::info("average " + string::to_string(num) + " images in " +
                          string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    } else if (Options::Operator::Diff == args.op) {
        if (uint32_t const num = op::difference(items, args, scene, resources.threads()); num) {
            logging::info("diff " + string::to_string(num) + " images in " +
                          string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    } else if (Options::Operator::Cat == args.op) {
        if (uint32_t const num = op::concatenate(items, args, pipeline, scene, resources.threads());
            num) {
            logging::info("cat " + string::to_string(num) + " images in " +
                          string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    } else if (Options::Operator::Sub == args.op) {
        if (uint32_t const num = op::sub(items, args, scene, resources.threads()); num) {
            logging::info("subtract " + string::to_string(num) + " images in " +
                          string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    }

    return 0;
}

void load_pipeline(std::istream& stream, std::string_view take_name, Pipeline& pipeline,
                   Resources& resources) {
    std::string error;
    auto const  root = json::parse(stream, error);
    if (!root.HasParseError()) {
        logging::push_error(error);
        return;
    }

    for (auto& n : root.GetObject()) {
        if ("camera" == n.name) {
            take::Loader::load_camera(n.value, nullptr, pipeline.camera);
        } else if ("post" == n.name) {
            std::string_view const take_mount_folder = string::parent_directory(take_name);

            auto& filesystem = resources.filesystem();

            filesystem.push_mount(take_mount_folder);

            take::Loader::load_postprocessors(n.value, resources, pipeline.pp);

            filesystem.pop_mount();
        }
    }
}
