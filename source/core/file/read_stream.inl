#ifndef SU_CORE_READ_STREAM_INL
#define SU_CORE_READ_STREAM_INL

#include "read_stream.hpp"

namespace file {

template <class Filebuffer>
Read_stream<Filebuffer>::Read_stream() : __istream_type(&stream_buffer_) {}

template <class Filebuffer>
void Read_stream<Filebuffer>::open(std::istream* stream, uint32_t read_size, char* read_buffer,
                                   uint32_t size, char* buffer) {
    if (!stream_buffer_.open(stream, read_size, read_buffer, size, buffer)) {
        __istream_type::setstate(std::ios_base::failbit);
    }
}

template <class Filebuffer>
void Read_stream<Filebuffer>::close() {
    if (!stream_buffer_.close()) {
        __istream_type::setstate(std::ios_base::failbit);
    }
}

}  // namespace file

#endif
