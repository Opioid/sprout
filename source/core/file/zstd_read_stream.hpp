#ifndef SU_CORE_ZSTD_READ_STREAM_HPP
#define SU_CORE_ZSTD_READ_STREAM_HPP

#include "zstd/zstd.h"

#include <istream>
#include <streambuf>

namespace file::zstd {

class Filebuffer final : public std::basic_streambuf<char, std::char_traits<char>> {
  public:
    using char_type   = char;
    using traits_type = std::char_traits<char>;
    using int_type    = traits_type::int_type;
    using pos_type    = traits_type::pos_type;
    using off_type    = traits_type::off_type;

    using __streambuf_type = std::basic_streambuf<char, std::char_traits<char>>;

    Filebuffer(uint32_t read_size, char* read_buffer, uint32_t size, char* buffer);

    ~Filebuffer() final;

    bool is_open() const;

    Filebuffer* open(std::istream* stream);

    void close();

    static uint32_t read_buffer_size();

    static uint32_t write_buffer_size();

  protected:
    int_type underflow() final;

    pos_type seekpos(pos_type pos, std::ios_base::openmode) final;

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode mode) final;

    int_type overflow(int_type c = traits_type::eof()) final;

    int sync() final;

    std::streamsize showmanyc() final;

  private:
    void restart_zstd_stream();

    pos_type data_start_;
    pos_type total_out_;

    std::istream* stream_;

    ZSTD_DStream* zstd_stream_;

    ZSTD_inBuffer zstd_input_;

    uint32_t read_buffer_size_;
    uint32_t buffer_size_;

    char_type* read_buffer_;
    char_type* buffer_;
};

}  // namespace file::zstd

#endif
