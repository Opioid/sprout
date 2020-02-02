#ifndef SU_CORE_FILE_SYSTEM_HPP
#define SU_CORE_FILE_SYSTEM_HPP

#include "base/memory/unique.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace file {

using Stream_ptr = memory::Unique_ptr<std::istream>;

class System {
  public:
    Stream_ptr read_stream(std::string_view name) const;

    Stream_ptr read_stream(std::string_view name, std::string& resolved_name) const;

    void push_mount(std::string_view folder);

    void pop_mount();

  private:
    std::istream* open_read_stream(std::string_view name, std::string& resolved_name) const;

    std::vector<std::string> mount_folders_;
};

}  // namespace file

extern template class memory::Unique_ptr<std::istream>;

#endif
