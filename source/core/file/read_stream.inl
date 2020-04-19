#ifndef SU_CORE_READ_STREAM_INL
#define SU_CORE_READ_STREAM_INL

#include "read_stream.hpp"

namespace file {

template <class Filebuffer>
Read_stream<Filebuffer>::Read_stream() : __istream_type(&stream_buffer_) {}

template <class Filebuffer>
Read_stream<Filebuffer>::Read_stream(std::istream* stream) : __istream_type(&stream_buffer_) {
    open(stream);
}

template <class Filebuffer>
const Filebuffer* Read_stream<Filebuffer>::rdbuf() const {
    return &stream_buffer_;
}

template <class Filebuffer>
Filebuffer* Read_stream<Filebuffer>::rdbuf() {
    return &stream_buffer_;
}

template <class Filebuffer>
bool Read_stream<Filebuffer>::is_open() const {
    return rdbuf()->is_open();
}

template <class Filebuffer>
void Read_stream<Filebuffer>::open(std::istream* stream) {
    if (!rdbuf()->open(stream)) {
        __istream_type::setstate(std::ios_base::failbit);
    }
}

template <class Filebuffer>
void Read_stream<Filebuffer>::close() {
    if (!rdbuf()->close()) {
        __istream_type::setstate(std::ios_base::failbit);
    }
}

}  // namespace file

#endif
