#include "difference_report_org.hpp"
#include "difference_item.hpp"
#include "item.hpp"

#include <cmath>
#include <ostream>

namespace op {

void write_difference_report_org(std::vector<Item> const&            items,
                                 std::vector<Difference_item> const& dif_items,
                                 std::ostream&                       stream) noexcept {
    stream << "* Difference " << items[0].name << "\n\n";

    stream << "** Summary\n\n";
    write_difference_summary_table_org(dif_items, stream);

    stream << "\n";
    stream << "** Images\n\n";

    for (size_t i = 0, len = dif_items.size(); i < len; ++i) {
        auto const& item = dif_items[i];

        stream << "*** " << items[i + 1].name << "\n\n";

        stream << "RMSE: " << round(item.rmse(), 4) << "\n";
        stream << "PSNR: " << round(item.psnr(), 2) << " dB\n\n";

        stream << "[[file:" << item.name() << "]]\n\n";
    }
}

void write_difference_summary_table_org(std::vector<Difference_item> const& items,
                                        std::ostream&                       stream) noexcept {
    size_t max_name_len = 0;

    for (auto const& i : items) {
        max_name_len = std::max(max_name_len, i.name().size());
    }

    if (items.size() > 1) {
        // /Average/
        size_t constexpr len = 9;
        max_name_len         = std::max(max_name_len, len);
    } else {
        // Image
        size_t constexpr len = 5;
        max_name_len         = std::max(max_name_len, len);
    }

    max_name_len += 2;

    stream << "| Image";

    {
        size_t const padding = max_name_len - 6;

        for (size_t i = 0; i < padding; ++i) {
            stream << " ";
        }
    }

    stream << "|   RMSE | PSNR     |\n";

    stream << "|";

    for (size_t i = 0; i < max_name_len; ++i) {
        stream << "-";
    }

    stream << "+--------+----------|\n";

    if (items.size() > 1) {
        stream << "| /Average/";

        size_t const padding = max_name_len - 10;

        for (size_t i = 0; i < padding; ++i) {
            stream << " ";
        }

        float average_rmse = 0.f;
        float average_psnr = 0.f;

        for (auto const& i : items) {
            average_rmse += i.rmse();
            average_psnr += i.psnr();
        }

        float const num_items = static_cast<float>(items.size());

        average_rmse /= num_items;
        average_psnr /= num_items;

        stream << "| " << round(average_rmse, 4) << " | " << round(average_psnr, 2) << " dB |\n";
    }

    for (auto const& i : items) {
        stream << "| " << i.name();

        size_t const padding = (max_name_len - i.name().size() - 1);

        for (size_t i = 0; i < padding; ++i) {
            stream << " ";
        }

        stream << "| " << round(i.rmse(), 4) << " | " << round(i.psnr(), 2) << " dB |\n";
    }
}

}  // namespace op
