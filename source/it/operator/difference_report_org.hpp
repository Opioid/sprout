#ifndef SU_IT_OPERATOR_DIFFERENCE_REPORT_ORG_HPP
#define SU_IT_OPERATOR_DIFFERENCE_REPORT_ORG_HPP

#include <iosfwd>
#include <vector>

struct Item;

namespace op {

class Difference_item;

void write_difference_report_org(std::vector<Item> const&            items,
                                 std::vector<Difference_item> const& dif_items, float max_dif,
                                 std::ostream& stream) noexcept;

void write_difference_summary_org(std::vector<Item> const&            items,
                                  std::vector<Difference_item> const& dif_items, float max_dif,
                                  std::ostream& stream) noexcept;

}  // namespace op

#endif
