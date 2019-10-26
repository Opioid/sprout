#include "base/chrono/chrono.hpp"
#include "base/platform/platform.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/logging.hpp"
#include "core/resource/resource_manager.inl"
#include "item.hpp"
#include "operator/average.hpp"
#include "operator/concatenate.hpp"
#include "operator/difference.hpp"
#include "options/options.hpp"

using namespace it::options;

void comparison(std::vector<Item> const& items) noexcept;

int main(int argc, char* argv[]) noexcept {
    auto const total_start = std::chrono::high_resolution_clock::now();

    logging::init(logging::Type::Std_out);

    //    logging::info("Welcome to di (" + platform::build() + " - " + platform::revision() +
    //    ")!");

    auto const args = it::options::parse(argc, argv);

    logging::set_verbose(args.verbose);

    if (args.images.empty()) {
        logging::error("No images specified.");
        return 1;
    }

    file::System file_system;

    uint32_t const available_threads = std::max(std::thread::hardware_concurrency(), 1u);

    uint32_t num_workers;
    if (args.threads <= 0) {
        int32_t const num_threads = static_cast<int32_t>(available_threads) + args.threads;

        num_workers = uint32_t(std::max(num_threads, 1));
    } else {
        num_workers = std::min(available_threads, uint32_t(std::max(args.threads, 1)));
    }

    //   logging::info("#Threads " + string::to_string(num_workers));

    thread::Pool threads(num_workers);

    resource::Manager resource_manager(file_system, threads);

    image::Provider image_provider;
    resource_manager.register_provider(image_provider);

    texture::Provider texture_provider(false);
    resource_manager.register_provider(texture_provider);

    std::vector<Item> items;
    items.reserve(args.images.size());

    memory::Variant_map options;
    options.set("usage", texture::Provider::Usage::Color_with_alpha);

    uint32_t slot = Options::Operator::Diff == args.op ? 0xFFFFFFFF : 0;
    for (auto& i : args.images) {
        if (Texture const* image = resource_manager.load<Texture>(i, options).ptr; image) {
            std::string const name_out = slot < args.outputs.size() ? args.outputs[slot] : "";

            items.emplace_back(Item{i, name_out, image});
        }

        ++slot;
    }

    if (items.empty()) {
        return 1;
    }

    if (Options::Operator::Average == args.op) {
        if (uint32_t const num = op::average(items, args, resource_manager.threads()); num) {
            logging::verbose("average " + string::to_string(num) + " images in " +
                             string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    } else if (Options::Operator::Diff == args.op || Options::Operator::Undefined == args.op) {
        if (uint32_t const num = op::difference(items, args, resource_manager.threads()); num) {
            logging::verbose("diff " + string::to_string(num) + " images in " +
                             string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    } else if (Options::Operator::Cat == args.op) {
        if (uint32_t const num = op::concatenate(items, args.concat_num_per_row, args.clip,
                                                 resource_manager.threads());
            num) {
            logging::verbose("cat " + string::to_string(num) + " images in " +
                             string::to_string(chrono::seconds_since(total_start)) + " s");
        }
    }

    //   comparison(items);

    return 0;
}

void comparison(std::vector<Item> const& items) noexcept {
    for (auto const& item : items) {
        int2 const d = item.image->dimensions_2();

        int32_t const num_pixels = d[0] * d[1];

        uint32_t count = 0;

        for (int32_t i = 0; i < num_pixels; ++i) {
            float3 const v = item.image->at_3(i);

            float const m = max_component(v);

            if (m >= 1.f) {
                ++count;
            }
        }

        logging::info("%S has " + string::to_string(count) + " pixels >= 1.f", item.name);
    }
}
