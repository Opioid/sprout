#ifndef SU_IT_OPERATOR_DIFFERENCE_REPORT_HTML_HPP
#define SU_IT_OPERATOR_DIFFERENCE_REPORT_HTML_HPP

#include <iosfwd>
#include <vector>

struct Item;

namespace op {

class Difference_item;

void write_difference_report_html(std::vector<Item> const&            items,
                                  std::vector<Difference_item> const& dif_items, float max_dif,
                                  std::ostream& stream);

}  // namespace op

#endif
