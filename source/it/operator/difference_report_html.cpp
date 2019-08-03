#include "difference_report_html.hpp"
#include "difference_item.hpp"
#include "difference_report_org.hpp"
#include "item.hpp"

#include <ostream>

namespace op {

void write_difference_report_html(std::vector<Item> const&            items,
                                  std::vector<Difference_item> const& dif_items,
                                  std::ostream&                       stream) noexcept {
    stream << "<!DOCTYPE html>\n";
    stream << "<html>\n";

    stream << "<head>\n";
    stream << "<meta charset=\"utf-8\">\n";
    stream << "<style>\n";

    stream << R"(
    body {
        background-color: #272822;
        color: #e6e5e2;
        font-size: large;
    }

    .container {

    }
    .one {
        float: left;
        padding: 8px;
    }
    .two {
        padding: 8px;
    }
    )";

    stream << "</style>\n";

    stream << "<title> Difference " << items[0].name << "</title>\n";
    stream << "</head>\n";

    stream << "<body>\n";
    stream << "<h1>Difference " << items[0].name << "</h1>\n";

    stream << "<h2>Summary</h2>\n";

    stream << "<table>\n";
    stream << "<tr>\n";
    stream << "<th>Image</th><th>RMSE</th><th>PSNR</th>\n";
    stream << "</tr>\n";

    if (dif_items.size() > 1) {
        float average_rmse = 0.f;
        float average_psnr = 0.f;

        for (auto const& i : dif_items) {
            average_rmse += i.rmse();
            average_psnr += i.psnr();
        }

        float const num_items = static_cast<float>(dif_items.size());

        average_rmse /= num_items;
        average_psnr /= num_items;

        stream << "<tr>\n";
        stream << "<td><i>Average</i></td><td>" << round(average_rmse, 4) << "</td>";
        stream << "<td>" << round(average_psnr, 2) << " dB</td>\n";
        stream << "</tr>\n";
    }

    for (auto const& i : dif_items) {
        stream << "<tr>\n";
        stream << "<td>" << i.name() << "</td><td>" << round(i.rmse(), 4) << "</td>";
        stream << "<td>" << round(i.psnr(), 2) << " dB</td>\n";
        stream << "</tr>\n";
    }

    stream << "</table>\n";

    stream << "<h2>Images</h2>\n";

    for (size_t i = 0, len = dif_items.size(); i < len; ++i) {
        auto const& item = dif_items[i];

        std::string const source_name = items[i + 1].name;

        stream << "<h3>" << source_name << "</h3>\n";

        stream << "<table>\n";
        stream << "<tr><td><b>RMSE:</b></td><td>" << round(item.rmse(), 4) << "</td></tr>";
        stream << "<tr><td><b>PSNR:</b></td><td>" << round(item.psnr(), 2) << " dB</td></tr>";
        stream << "</table>\n";

        stream << "<section class=\"container\">";

        stream << "<div class=\"one\">\n";
        stream << "<a href=\"" << source_name << "\">";

        stream << "<img src=\"" << source_name << "\" onmouseover=\"this.src='" << items[0].name
               << "';\" onmouseout=\"this.src='" << source_name << "';\" />\n";

        stream << "</div>\n";

        stream << "<div class=\"two\">\n";
        stream << "<a href=\"" << item.name() << "\">";
        stream << "<img src=\"" << item.name() << "\"></a>";
        stream << "</div>\n";
        stream << "</section>";
    }

    stream << "</body>\n";

    stream << "</html>\n";
}

}  // namespace op
