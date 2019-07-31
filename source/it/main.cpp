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
#include "difference.hpp"
#include "item.hpp"
#include "options/options.hpp"

using namespace it;

void comparison(std::vector<Item> const& items);

int main(int argc, char* argv[]) {
    auto const total_start = std::chrono::high_resolution_clock::now();

    logging::init(logging::Type::Std_out);

    //    logging::info("Welcome to di (" + platform::build() + " - " + platform::revision() +
    //    ")!");

    auto const args = options::parse(argc, argv);

    if (args.images.empty()) {
        logging::error("No images specified.");
        return 1;
    }

    file::System file_system;

    uint32_t const available_threads = std::max(std::thread::hardware_concurrency(), 1u);

    uint32_t num_workers;
    if (args.threads <= 0) {
        int32_t const num_threads = static_cast<int32_t>(available_threads) + args.threads;

        num_workers = static_cast<uint32_t>(std::max(num_threads, 1));
    } else {
        num_workers = std::min(available_threads, static_cast<uint32_t>(std::max(args.threads, 1)));
    }

    //   logging::info("#Threads " + string::to_string(num_workers));

    thread::Pool thread_pool(num_workers);

    resource::Manager resource_manager(file_system, thread_pool);

    Provider image_provider;
    resource_manager.register_provider(image_provider);

    texture::Provider texture_provider;
    resource_manager.register_provider(texture_provider);

    std::vector<Item> items;
    items.reserve(args.images.size());

    for (auto& i : args.images) {
        if (Texture const* image = resource_manager.load<Texture>(i); image) {
            items.emplace_back(Item{i, image});
        }
    }

    if (uint32_t num = difference(items, args.clamp, args.clip, resource_manager.thread_pool());
        num) {
        logging::info("diff " + string::to_string(num) + " images in " +
                      string::to_string(chrono::seconds_since(total_start)) + " s");
    }

    //   comparison(items);

    return 0;
}

void comparison(std::vector<Item> const& items) {
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
