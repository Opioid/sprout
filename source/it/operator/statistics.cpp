#include "statistics.hpp"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "item.hpp"

#include <iomanip>
#include <sstream>

#include <iostream>
#include "base/math/print.hpp"

namespace op {

float luminance_sRGB(float3 const& linear) noexcept;

float2 average_and_max_luminance(Texture const* image);

std::string print_histogram(Item const& item) noexcept;

struct Histogram {
    static uint32_t constexpr Num_buckets = 64;

    uint32_t buckets[Num_buckets];

    float max_value;

    Histogram(float max_value) : max_value(max_value) {
        for (uint32_t& b : buckets) {
            b = 0;
        }
    }

    void insert(float value) noexcept {
        uint32_t const i = uint32_t((value / max_value) * float(Num_buckets - 1));

        ++buckets[i];
    }

    uint32_t max() {
        uint32_t m = 0;

        for (uint32_t b : buckets) {
            m = std::max(b, m);
        }

        return m;
    }
};

uint32_t statistics(std::vector<Item> const& items, it::options::Options const& options,
                    thread::Pool& threads) noexcept {
    for (auto const& i : items) {
    //    float2 const aml = average_and_max_luminance(i.image);

    //    std::cout << aml << std::endl;

        std::string const hist = print_histogram(i);

        logging::info(hist);
    }

    return 1;
}

float2 average_and_max_luminance(Texture const* image) {
    int32_t const len = image->volume();

    float const ilen = 1.f / float(len);

    float average = 0.f;
    float max     = 0.f;

    for (int32_t i = 0; i < len; ++i) {
        float const luminance = luminance_sRGB(image->at_3(i));

        average += ilen * luminance;

        max = std::max(luminance, max);
    }

    return float2(average, max);
}

std::string print_histogram(Item const& item) noexcept {
    Texture const* image = item.image;

    float2 const aml = average_and_max_luminance(image);

    Histogram hist(aml[1]);

    int32_t const len = image->volume();

    for (int32_t i = 0; i < len; ++i) {
        float const luminance = luminance_sRGB(image->at_3(i));

        hist.insert(luminance);
    }

    float const hist_max = float(hist.max());

    std::stringstream stream;

    static uint32_t Num_rows = 20;

    float const nl = hist_max / float(len);

    stream << std::fixed;
    stream << std::setprecision(1);

    for (uint32_t r = 0; r <= Num_rows; ++r) {
        if (0 == r % 6) {
            int32_t const pp = int32_t(1000 * float(Num_rows - r) / float(Num_rows) * nl + 0.5f);

            if (pp < 100) {
                stream << " ";
            }

            stream << (float(pp) / 10.f) << "% +";
        } else {
            stream << "      |";
        }

        if (0 == r) {
            for (uint32_t i = 0; i < Histogram::Num_buckets; ++i) {
                stream << "-";
            }

            stream << "+\n";
            continue;
        }

        float const bar      = float(Num_rows - r) / float(Num_rows);
        float const half_bar = (float(Num_rows - r) - 0.5f) / (Num_rows);

        for (uint32_t i = 0; i < Histogram::Num_buckets; ++i) {
            float const percent = float(hist.buckets[i]) / hist_max;

            if (percent > bar) {
                stream << ':';//  char(219);
            } else if (percent > half_bar) {
                stream << '.';// char(220);
            } else {
                stream << " ";
            }
        }

        stream << "|\n";
    }

    stream << "      ";

    for (uint32_t i = 0; i <= Histogram::Num_buckets + 1; ++i) {
        if (0 == i || (i > 1 && 0 == (i - 1) % 16)) {
            stream << "+";
        } else {
            stream << "-";
        }
    }

    stream << "\n     ";

    stream << std::setprecision(2);

    int32_t grace = 0;

    for (uint32_t i = 0; i < 66; ++i) {
        if (0 == i) {
            stream << float(0.f);
            grace = 3;
        } else if (i > 1 && 0 == (i) % 16) {
            stream << float(i) / Histogram::Num_buckets;
            grace = 4;
        } else if (grace <= 0) {
            stream << " ";
        }

        --grace;
    }

    return stream.str();
}

float luminance_sRGB(float3 const& linear) noexcept {
    return spectrum::linear_to_gamma_sRGB(spectrum::luminance(linear));
}

}  // namespace op
