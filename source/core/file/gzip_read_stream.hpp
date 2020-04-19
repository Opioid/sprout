#ifndef SU_CORE_GZIP_READ_STREAM_HPP
#define SU_CORE_GZIP_READ_STREAM_HPP

#include "miniz/miniz.h"

#include <istream>
#include <streambuf>

namespace file::gzip {

class Filebuffer final : public std::basic_streambuf<char, std::char_traits<char>> {
  public:
    using char_type   = char;
    using traits_type = std::char_traits<char>;
    using int_type    = traits_type::int_type;
    using pos_type    = traits_type::pos_type;
    using off_type    = traits_type::off_type;

    using __streambuf_type = std::basic_streambuf<char, std::char_traits<char>>;

    Filebuffer();

    ~Filebuffer() final;

    bool is_open() const;

    //	Filebuffer* open(char const* filename, std::ios_base::openmode mode);

    Filebuffer* open(std::istream* stream);

    Filebuffer* close();

  protected:
    int_type underflow() final;

    pos_type seekpos(pos_type pos, std::ios_base::openmode) final;

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode mode) final;

    int_type overflow(int_type c = traits_type::eof()) final;

    int sync() final;

    std::streamsize showmanyc() final;

  private:
    bool init_z_stream();

    pos_type data_start_;

    std::istream* stream_;

    mz_stream z_stream_;

    static uint32_t constexpr Buffer_size = 8192;

    char_type read_buffer_[Buffer_size];

    char_type buffer_[Buffer_size];
};

}  // namespace file::gzip

#endif
