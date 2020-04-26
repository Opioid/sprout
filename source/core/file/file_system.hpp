#ifndef SU_CORE_FILE_SYSTEM_HPP
#define SU_CORE_FILE_SYSTEM_HPP

#include "gzip_read_stream.hpp"
#include "read_stream.hpp"
#include "zstd_read_stream.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace file {

class System {
  public:
    System();

    ~System();

    std::istream& read_stream(std::string_view name);

    std::istream& read_stream(std::string_view name, std::string& resolved_name);

    std::istream& string_stream(std::string const& string);

    void close_stream(std::istream& stream);

    void push_mount(std::string_view folder);

    void pop_mount();

  private:
    std::istream& open_read_stream(std::string_view name, std::string& resolved_name);

    void allocate_buffers(uint32_t read_size, uint32_t size);

    std::ifstream stream_;

    Read_stream<gzip::Filebuffer> gzip_stream_;
    Read_stream<zstd::Filebuffer> zstd_stream_;
    std::istringstream            str_stream_;

    uint32_t read_buffer_size_;
    uint32_t buffer_size_;

    char* read_buffer_;
    char* buffer_;

    std::vector<std::string> mount_folders_;
};

}  // namespace file

#endif
