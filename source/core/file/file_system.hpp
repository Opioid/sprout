#ifndef SU_CORE_FILE_SYSTEM_HPP
#define SU_CORE_FILE_SYSTEM_HPP

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace file {

class System {
  public:
    using Stream_ptr = std::unique_ptr<std::istream>;

    Stream_ptr read_stream(std::string_view name) const;

    Stream_ptr read_stream(std::string_view name, std::string& resolved_name) const;

    void push_mount(std::string_view folder);

    void pop_mount();

  private:
    std::istream* open_read_stream(std::string_view name, std::string& resolved_name) const;

    std::vector<std::string> mount_folders_;
};

}  // namespace file

#endif
