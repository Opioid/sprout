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
    System();

    ~System();

    Stream_ptr read_stream(std::string_view name);

    Stream_ptr read_stream(std::string_view name, std::string& resolved_name);

    void push_mount(std::string_view folder);

    void pop_mount();

  private:
    std::istream* open_read_stream(std::string_view name, std::string& resolved_name);

    void allocate_buffers(uint32_t read_size, uint32_t size);

    std::vector<std::string> mount_folders_;

    uint32_t read_buffer_size_;
    uint32_t buffer_size_;

    char* read_buffer_;
    char* buffer_;
};

}  // namespace file

extern template class memory::Unique_ptr<std::istream>;

#endif
