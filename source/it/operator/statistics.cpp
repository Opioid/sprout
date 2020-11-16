#include "statistics.hpp"
#include "base/spectrum/rgb.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "item.hpp"
#include "options/options.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace op {

float luminance_gamma_sRGB(float3_p linear);

struct Luminance {
    float avg;
    float max;
};

Luminance average_and_max_luminance(Texture const* image);

void write_histogram(Item const& item, std::ostream& stream);

class Histogram {
  public:
    static uint32_t constexpr Num_buckets = 64;

    Histogram(float max_value)
        : max_value_(max_value), num_buckets_(uint32_t(std::ceil(max_value * 4.f)) * Num_buckets) {
        buckets_ = new uint32_t[num_buckets_];

        for (uint32_t i = 0, len = num_buckets_; i < len; ++i) {
            buckets_[i] = 0;
        }
    }

    ~Histogram() {
        delete[] buckets_;
    }

    void insert(float value) {
        uint32_t const i = uint32_t(std::lrint((value / max_value_) * float(num_buckets_ - 1)));

        ++buckets_[i];
    }

    uint32_t count(uint32_t id) const {
        uint32_t const factor = num_buckets_ / Num_buckets;

        uint32_t b = 0;

        uint32_t const begin = id * factor;
        uint32_t const end   = (id + 1) * factor;

        for (uint32_t i = begin; i < end; ++i) {
            b = std::max(buckets_[i], b);
        }

        return b;
    }

    uint32_t max() const {
        uint32_t m = 0;

        for (uint32_t i = 0, len = num_buckets_; i < len; ++i) {
            m = std::max(buckets_[i], m);
        }

        return m;
    }

  private:
    float max_value_;

    uint32_t num_buckets_;

    uint32_t* buckets_;
};

uint32_t statistics(std::vector<Item> const& items, it::options::Options const& options,
                    Threads& /*threads*/) {
    std::ostringstream stream;

    for (auto const& i : items) {
        if (items.size() > 1) {
            stream << i.name << "\n";
        }

        write_histogram(i, stream);

        if (items.size() > 1) {
            stream << "\n";
        }
    }

    if ("." == options.statistics || options.statistics.empty()) {
        logging::info(stream.str());
    } else if (!options.statistics.empty()) {
        std::ofstream fstream(options.statistics);

        fstream << stream.str();
    }

    return 1;
}

Luminance average_and_max_luminance(Texture const* image) {
    int32_t const len = image->volume();

    float const ilen = 1.f / float(len);

    float average = 0.f;
    float max     = 0.f;

    auto const d = image->dimensions();

    for (int32_t z = 0; z < d[2]; ++z) {
        for (int32_t y = 0; y < d[1]; ++y) {
            for (int32_t x = 0; x < d[0]; ++x) {
                float const luminance = luminance_gamma_sRGB(image->at_3(x, y, z));

                average += ilen * luminance;

                max = std::max(luminance, max);
            }
        }
    }

    return {average, max};
}

void write_histogram(Item const& item, std::ostream& stream) {
    Texture const* image = item.image;

    auto const [avg_l, max_l] = average_and_max_luminance(image);

    Histogram hist(max_l);

    auto const d = image->dimensions();

    for (int32_t z = 0; z < d[2]; ++z) {
        for (int32_t y = 0; y < d[1]; ++y) {
            for (int32_t x = 0; x < d[0]; ++x) {
                float const luminance = luminance_gamma_sRGB(image->at_3(x, y, z));

                hist.insert(luminance);
            }
        }
    }

    float const hist_max = float(hist.max());

    stream << std::fixed;
    stream << std::setprecision(3);

    stream << "Luminance: avg " << avg_l << "; max " << max_l;

    stream << "                           sRGB gamma space\n\n";

    static uint32_t Num_rows = 16;

    float const nl = hist_max / float(image->volume());

    stream << std::setprecision(1);

    for (uint32_t r = 0; r <= Num_rows; ++r) {
        if (0 == r % 4 && r < Num_rows) {
            float const pp = 100.f * float(Num_rows - r) / float(Num_rows) * nl;

            if (pp < 100.f) {
                stream << " ";
            }

            if (pp < 10.f) {
                stream << " ";
            }

            stream << pp << "% +";
        } else {
            //   stream << "      |";
            stream << "        ";
        }

        if (0 == r) {
            for (uint32_t i = 1; i < Histogram::Num_buckets; ++i) {
                if (i > 1 && 0 == (i - 1) % 16) {
                    stream << "+";
                } else {
                    stream << " ";
                }
            }

            stream << " +\n";
            continue;
        }

        float const bar_0 = (float(Num_rows - r + 1) - 0.25f) / float(Num_rows);
        float const bar_1 = (float(Num_rows - r + 1) - 0.5f) / float(Num_rows);
        float const bar_2 = (float(Num_rows - r + 1) - 0.75f) / float(Num_rows);
        float const bar_3 = (float(Num_rows - r + 1) - 1.f) / float(Num_rows);

        for (uint32_t i = 0; i < Histogram::Num_buckets; ++i) {
            float const percent = float(hist.count(i)) / hist_max;

            if (percent >= bar_0) {
                stream << '|';
            } else if (percent >= bar_1) {
                stream << 'i';
            } else if (percent >= bar_2) {
                stream << ':';
            } else if (percent >= bar_3) {
                stream << '.';
            } else {
                stream << " ";
            }
        }

        if (0 == r % 4 && r < Num_rows) {
            stream << "+";
        } else {
            //    stream << "|";
            stream << " ";
        }

        stream << "\n";
    }

    stream << "  0.0% ";

    for (uint32_t i = 0; i <= Histogram::Num_buckets + 1; ++i) {
        if (0 == i || (i > 1 && 0 == (i - 1) % 16)) {
            stream << "+";
        } else {
            stream << " ";
        }
    }

    stream << "\n      ";

    stream << std::setprecision(2);

    int32_t grace = 0;

    for (uint32_t i = 0; i < 66; ++i) {
        if (0 == i) {
            stream << float(0.f);
            grace = 3;
        } else if (i > 1 && 0 == (i) % 16) {
            stream << max_l * float(i) / float(Histogram::Num_buckets);
            grace = 4;
        } else if (grace <= 0) {
            stream << " ";
        }

        --grace;
    }
}

static inline float linear_to_gamma_sRGB_unbounded(float c) {
    if (c <= 0.f) {
        return 0.f;
    }

    if (c < 0.0031308f) {
        return 12.92f * c;
    }

    if (c < 1.f) {
        return 1.055f * std::pow(c, 0.41666f) - 0.055f;
    }

    return std::pow(c, 0.41666f);
}

float luminance_gamma_sRGB(float3_p linear) {
    return /*linear_to_gamma_sRGB_unbounded*/ (spectrum::luminance(linear));
}

}  // namespace op
