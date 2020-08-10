#include "difference.hpp"
#include "base/memory/array.inl"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/texture/texture.inl"
#include "core/logging/logging.hpp"
#include "difference_item.hpp"
#include "difference_report_html.hpp"
#include "difference_report_org.hpp"
#include "item.hpp"
#include "options/options.hpp"

#include <fstream>
#include <sstream>

using namespace image;

using Texture = texture::Texture;

namespace op {

uint32_t difference(std::vector<Item> const& items, it::options::Options const& options,
                    thread::Pool& threads) {
    if (items.size() < 2) {
        logging::error("Need at least 2 images for diff.");
        return 0;
    }

    Texture const& reference = items[0].image;

    int2 const dimensions = reference.dimensions().xy();

    std::vector<Difference_item> candidates;
    candidates.reserve(items.size() - 1);

    for (size_t i = 1, len = items.size(); i < len; ++i) {
        Item const& item = items[i];

        if (item.image.dimensions().xy() != dimensions) {
            logging::error("%S does not match reference resolution", item.name);
            continue;
        }

        candidates.emplace_back(item);
    }

    memory::Array<Scratch> scratch(threads.num_threads(), Scratch{0.f, 0.f, 0.f});

    float max_dif = 0.f;

    for (auto& c : candidates) {
        c.calculate_difference(reference, scratch.data(), options.clamp, options.clip, threads);

        max_dif = std::max(c.max_dif(), max_dif);
    }

    if ("." == options.report) {
        std::ostringstream stream;

        write_difference_summary_org(items, candidates, max_dif, stream);

        logging::info(stream.str());
    } else if (!options.report.empty()) {
        std::ofstream stream(options.report);

        std::string_view const suffix = string::suffix(options.report);

        if ("htm" == suffix || "html" == suffix) {
            write_difference_report_html(items, candidates, max_dif, stream);
        } else {
            write_difference_report_org(items, candidates, max_dif, stream);
        }
    }

    if (!options.no_export) {
        if (options.max_dif > 0.f) {
            max_dif = options.max_dif;
        }

        for (auto const& c : candidates) {
            encoding::png::Writer::write_heatmap(c.name(), c.difference(), dimensions, max_dif,
                                                 threads);
        }
    }

    return uint32_t(candidates.size()) + 1;
}

}  // namespace op
