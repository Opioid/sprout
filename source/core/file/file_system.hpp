#ifndef SU_CORE_FILE_SYSTEM_HPP
#define SU_CORE_FILE_SYSTEM_HPP

#include <iosfwd>
#include <string>
#include <vector>
#include "base/memory/unique.hpp"

namespace file {

using Stream_ptr = memory::Unique_ptr<std::istream>;

class System {
  public:
    Stream_ptr read_stream(std::string_view name) const noexcept;

    Stream_ptr read_stream(std::string_view name, std::string& resolved_name) const noexcept;

    void push_mount(std::string_view folder) noexcept;

    void pop_mount() noexcept;

  private:
    std::istream* open_read_stream(std::string_view name, std::string& resolved_name) const
        noexcept;

    std::vector<std::string> mount_folders_;
};

}  // namespace file

extern template class memory::Unique_ptr<std::istream>;

#endif
