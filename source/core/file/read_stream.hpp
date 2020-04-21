#ifndef SU_CORE_READ_STREAM_HPP
#define SU_CORE_READ_STREAM_HPP

#include <istream>
#include <streambuf>

namespace file {

template <class Filebuffer>
class Read_stream : public std::basic_istream<char, std::char_traits<char>> {
  public:
    using char_type   = char;
    using traits_type = std::char_traits<char>;
    using int_type    = traits_type::int_type;
    using pos_type    = traits_type::pos_type;
    using off_type    = traits_type::off_type;

    using __istream_type = std::basic_istream<char, std::char_traits<char>>;

    Read_stream();

    explicit Read_stream(std::istream* stream);

    const Filebuffer* rdbuf() const;

    Filebuffer* rdbuf();

    bool is_open() const;

    void open(std::istream* stream);

    void close();

  private:
    Filebuffer stream_buffer_;
};

}  // namespace file

#endif
