#ifndef SU_CORE_FILE_SYSTEM_HPP
#define SU_CORE_FILE_SYSTEM_HPP

#include "gzip_read_stream.hpp"
#include "read_stream.hpp"

#ifdef SU_ZSTD
#include "zstd_read_stream.hpp"
#endif

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace file {

class System {
  public:
    class Stream_ptr {
      public:
        friend System;

        enum class Type { Uncompressed, GZIP, ZSTD, String, Invalid };

        Stream_ptr(System& system, Type type);

        Stream_ptr(Stream_ptr&& other) noexcept;

        ~Stream_ptr();

        operator bool() const;

        bool operator!() const;

        std::istream& operator*() const;

        void close();

      private:
        System& system_;

        Type type_;
    };

    System();

    ~System();

    Stream_ptr read_stream(std::string_view name);

    Stream_ptr read_stream(std::string_view name, std::string& resolved_name);

    Stream_ptr string_stream(std::string const& string);

    std::istream& stream(Stream_ptr const& ptr);

    void close(Stream_ptr& stream);

    void push_mount(std::string_view folder);

    void pop_mount();

    void set_frame(uint32_t frame);

  private:
    std::istream& open_read_stream(std::string_view name, std::string& resolved_name);

    void allocate_buffers(uint32_t read_size, uint32_t size);

    std::ifstream stream_;

    Read_stream<gzip::Filebuffer> gzip_stream_;

#ifdef SU_ZSTD
    Read_stream<zstd::Filebuffer> zstd_stream_;
#endif

    std::istringstream str_stream_;

    uint32_t read_buffer_size_;
    uint32_t buffer_size_;

    char* read_buffer_;
    char* buffer_;

    std::vector<std::string> mount_folders_;

    std::string frame_string_;
};

}  // namespace file

#endif
