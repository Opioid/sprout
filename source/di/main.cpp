#include <algorithm>
#include "base/math/vector2.inl"
#include "base/platform/platform.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/file/file_system.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/image.hpp"
#include "core/image/image_provider.hpp"
#include "core/image/texture/texture.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/logging/logging.hpp"
#include "core/resource/resource_manager.inl"
#include "options/options.hpp"

using namespace di;
using namespace image;

using Texture = texture::Texture;

class Candidate {
  public:
    Candidate(std::string name, Texture const* image)
        : name_(name.substr(0, name.find_last_of('.')) + "_dif.png"), image_(image) {
        int2 const d = image->dimensions_2();
        difference_  = new float[d[0] * d[1]];
    }

    ~Candidate() {
        delete[] difference_;
    }

    std::string name() const {
        return name_;
    }

    float const* difference() const {
        return difference_;
    }

    float calculate_difference(Texture const* other) {
        int2 const    d         = image_->dimensions_2();
        int32_t const num_pixel = d[0] * d[1];

        float max_dif = 0.f;

        for (int32_t i = 0; i < num_pixel; ++i) {
            float3 const va = image_->at_3(i);
            float3 const vb = other->at_3(i);

            float const dif = distance(va, vb);

            difference_[i] = dif;

            max_dif = std::max(max_dif, dif);
        }

        return max_dif;
    }

  private:
    std::string name_;

    Texture const* image_;

    float* difference_;
};

int main(int argc, char* argv[]) {
    logging::init(logging::Type::Std_out);

    //    logging::info("Welcome to di (" + platform::build() + " - " + platform::revision() +
    //    ")!");

    auto const args = options::parse(argc, argv);

    if (args.reference.empty()) {
        logging::error("No reference image specified.");
        return 1;
    }

    if (args.images.empty()) {
        logging::error("No comparison images specified.");
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

    Texture* reference = resource_manager.load<Texture>(args.reference);

    if (!reference) {
        logging::error("Could not load reference image");
        return 1;
    }

    int2 const dimensions = reference->dimensions_2();

    std::vector<Candidate> candidates;
    candidates.reserve(args.images.size());

    for (auto i : args.images) {
        Texture* image = resource_manager.load<Texture>(i);

        if (!image) {
            logging::error("Could not load comparison image");
            continue;
        }

        if (image->dimensions_2() != dimensions) {
            logging::error("Image resolutions do no match");
            continue;
        }

        candidates.emplace_back(i, image);
    }

    float max_dif = 0.f;

    for (auto& c : candidates) {
        max_dif = std::max(c.calculate_difference(reference), max_dif);
    }

    for (auto const& c : candidates) {
        encoding::png::Writer::write_heatmap(c.name(), c.difference(), dimensions, max_dif);
    }

    return 0;
}
