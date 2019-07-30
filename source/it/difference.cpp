#include "difference.hpp"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/texture/texture.inl"
#include "core/resource/resource_manager.inl"

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

    float calculate_difference(Texture const* other, float* max_difs, thread::Pool& pool) {
        int2 const d = image_->dimensions_2();

        int32_t const num_pixel = d[0] * d[1];

        struct Args {
            Texture const* image;
            Texture const* other;

            float* difference;
            float* max_difs;
        };

        Args args = Args{image_, other, difference_, max_difs};

        pool.run_range(
            [&args](uint32_t id, int32_t begin, int32_t end) {
                float max_dif = 0.f;

                for (int32_t i = begin; i < end; ++i) {
                    float3 const va = args.image->at_3(i);
                    float3 const vb = args.other->at_3(i);

                    float const dif = distance(va, vb);

                    args.difference[i] = dif;

                    max_dif = std::max(max_dif, dif);
                }

                args.max_difs[id] = max_dif;
            },
            0, num_pixel);

        float max_dif = max_difs[0];
        for (uint32_t i = 1, len = pool.num_threads(); i < len; ++i) {
            max_dif = std::max(max_dif, max_difs[i]);
        }

        return max_dif;
    }

  private:
    std::string name_;

    Texture const* image_;

    float* difference_;
};

uint32_t difference(std::vector<std::string> const& images, resource::Manager& manager) {
    Texture* reference = manager.load<Texture>(images[0]);

    if (!reference) {
        logging::error("Could not load reference image");
        return 0;
    }

    int2 const dimensions = reference->dimensions_2();

    std::vector<Candidate> candidates;
    candidates.reserve(images.size() - 1);

    for (size_t i = 1, len = images.size(); i < len; ++i) {
        std::string const name = images[i];

        Texture* image = manager.load<Texture>(name);

        if (!image) {
            logging::error("Could not load comparison image");
            continue;
        }

        if (image->dimensions_2() != dimensions) {
            logging::error("Image resolutions do no match");
            continue;
        }

        candidates.emplace_back(name, image);
    }

    thread::Pool& pool = manager.thread_pool();

    memory::Array<float> max_difs(pool.num_threads(), 0.f);

    float max_dif = 0.f;

    for (auto& c : candidates) {
        max_dif = std::max(c.calculate_difference(reference, max_difs.data(), pool), max_dif);
    }

    encoding::png::Writer writer(dimensions, false);

    for (auto const& c : candidates) {
        writer.write_heatmap(c.name(), c.difference(), dimensions, max_dif);
    }

    return static_cast<uint32_t>(candidates.size()) + 1;
}
