#ifndef SU_CORE_READ_STREAM_HPP
#define SU_CORE_READ_STREAM_HPP

#include <istream>
#include <streambuf>

namespace file {

template <class Filebuffer>
class Read_stream : public std::basic_istream<char, std::char_traits<char>> {
  public:
    using __istream_type = std::basic_istream<char, std::char_traits<char>>;

    Read_stream();

    void open(std::istream* stream, uint32_t read_size, char* read_buffer, uint32_t size,
              char* buffer);

    void close();

  private:
    Filebuffer stream_buffer_;
};

}  // namespace file

#endif
