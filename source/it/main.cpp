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
#include "options/options.hpp"

using namespace it;
using namespace image;

using Texture = texture::Texture;

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

    if (uint32_t num = difference(args.images, resource_manager); num) {
        logging::info("diff " + string::to_string(num) + " images in " +
                      string::to_string(chrono::seconds_since(total_start)) + " s");
    }

    return 0;
}
