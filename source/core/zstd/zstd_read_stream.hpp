#ifndef SU_CORE_ZSTD_READ_STREAM
#define SU_CORE_ZSTD_READ_STREAM

#include "zstd/zstd.h"

#include <istream>
#include <streambuf>

namespace zstd {

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
    void restart_zstd_stream();

    pos_type data_start_;

    std::istream* stream_;

    ZSTD_DStream* zstd_stream_;

    ZSTD_inBuffer zstd_input_;

    pos_type total_out_;

    uint32_t read_buffer_size_;
    uint32_t buffer_size_;

    char_type* read_buffer_;
    char_type* buffer_;
};

class Read_stream : public std::basic_istream<char, std::char_traits<char>> {
  public:
    using char_type   = char;
    using traits_type = std::char_traits<char>;
    using int_type    = traits_type::int_type;
    using pos_type    = traits_type::pos_type;
    using off_type    = traits_type::off_type;

    using __istream_type = std::basic_istream<char, std::char_traits<char>>;

    Read_stream();

    //	explicit Read_stream(std::string const& name,
    //						 std::ios_base::openmode mode =
    // std::ios_base::binary);

    //	explicit Read_stream(char const* name,
    //						 std::ios_base::openmode mode =
    // std::ios_base::binary);

    explicit Read_stream(std::istream* stream);

    const Filebuffer* rdbuf() const;

    Filebuffer* rdbuf();

    bool is_open() const;

    //	void open(char const* name, std::ios_base::openmode mode = std::ios_base::binary);

    void open(std::istream* stream);

    void close();

  private:
    Filebuffer stream_buffer_;
};

}  // namespace zstd

#endif
