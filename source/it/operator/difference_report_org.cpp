#include "difference_report_org.hpp"
#include "difference_item.hpp"
#include "item.hpp"

#include <algorithm>
//#include <cmath>
#include <ostream>

namespace op {

void write_difference_report_org(std::vector<Item> const&            items,
                                 std::vector<Difference_item> const& dif_items, float max_dif,
                                 std::ostream& stream) {
    stream << "* Difference " << items[0].name << "\n\n";

    stream << "** Summary\n\n";
    write_difference_summary_org(items, dif_items, max_dif, stream);

    stream << "\n";
    stream << "** Images\n\n";

    for (size_t i = 0, len = dif_items.size(); i < len; ++i) {
        auto const& item = dif_items[i];

        stream << "*** " << items[i + 1].name << "\n\n";

        stream << "RMSE: " << item.rmse() << "\n";
        stream << "PSNR: " << item.psnr() << " dB\n\n";

        stream << "[[file:" << item.name() << "]]\n\n";
    }
}

void write_difference_summary_org(std::vector<Item> const&            items,
                                  std::vector<Difference_item> const& dif_items, float max_dif,
                                  std::ostream& stream) {
    stream << "Max difference: " << max_dif << "\n\n";

    size_t max_name_len = 0;

    for (size_t i = 1, len = items.size(); i < len; ++i) {
        max_name_len = std::max(max_name_len, items[i].name.size());
    }

    // Image
    size_t constexpr image_len = 5;

    max_name_len = std::max(max_name_len, image_len);

    max_name_len += 2;

    stream << "| Image";

    {
        size_t const padding = max_name_len - 6;

        std::fill_n(std::ostreambuf_iterator<char>(stream), padding, ' ');
    }

    stream << "|   RMSE | PSNR     |\n";

    stream << "|";

    for (size_t i = 0; i < max_name_len; ++i) {
        stream << "-";
    }

    stream << "+--------+----------|\n";

    for (size_t i = 0, len = dif_items.size(); i < len; ++i) {
        auto const& item = dif_items[i];

        std::string const source_name = items[i + 1].name;

        stream << "| " << source_name;

        {
            size_t const padding = (max_name_len - source_name.size() - 1);

            std::fill_n(std::ostreambuf_iterator<char>(stream), padding, ' ');
        }

        stream << "| ";

        {
            auto const o = stream.tellp();

            stream << item.rmse();

            auto const n = stream.tellp();

            std::fill_n(std::ostreambuf_iterator<char>(stream), 7 - (n - o), ' ');
        }

        stream << "| ";

        {
            auto const o = stream.tellp();

            stream << item.psnr() << " dB";

            auto const n = stream.tellp();

            std::fill_n(std::ostreambuf_iterator<char>(stream), 9 - (n - o), ' ');
        }

        stream << "|\n";
    }
}

}  // namespace op
