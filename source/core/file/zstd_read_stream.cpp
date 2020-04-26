#include "zstd_read_stream.hpp"

#include <cstring>

namespace file::zstd {

Filebuffer::Filebuffer() : stream_(nullptr), zstd_stream_(nullptr) {}

Filebuffer::~Filebuffer() {
    close();
}

bool Filebuffer::is_open() const {
    return nullptr != stream_;
}

Filebuffer* Filebuffer::open(std::istream* stream, uint32_t read_size, char* read_buffer,
                             uint32_t size, char* buffer) {
    close();

    stream_           = stream;
    read_buffer_size_ = read_size;
    buffer_size_      = size;
    read_buffer_      = read_buffer;
    buffer_           = buffer;

    data_start_ = stream_->tellg();

    zstd_stream_ = ZSTD_createDCtx();

    restart_zstd_stream();

    return this;
}

Filebuffer* Filebuffer::close() {
    // Return failure if this file buf is closed already
    if (!is_open()) {
        return nullptr;
    }

    sync();

    ZSTD_freeDCtx(zstd_stream_);
    zstd_stream_ = nullptr;

    stream_ = nullptr;

    return this;
}

uint32_t Filebuffer::read_buffer_size() {
    return uint32_t(ZSTD_DStreamInSize());
}

uint32_t Filebuffer::write_buffer_size() {
    return uint32_t(ZSTD_DStreamOutSize());
}

// https://facebook.github.io/zstd/zstd_manual.html#Chapter9
// https://github.com/facebook/zstd/blob/dev/examples/streaming_decompression.c

Filebuffer::int_type Filebuffer::underflow() {
    if (!is_open()) {
        return traits_type::eof();
    }

    //   char_type* current = gptr();

    size_t uncompressed_bytes = 0;

    while (0 == uncompressed_bytes) {
        if (zstd_input_.pos >= zstd_input_.size) {
            stream_->read(read_buffer_, read_buffer_size_);

            uint32_t const read_bytes = *stream_ ? read_buffer_size_ : uint32_t(stream_->gcount());

            if (0 == read_bytes) {
                return traits_type::eof();
            }

            zstd_input_.size = read_bytes;
            zstd_input_.pos  = 0;
        }

        ZSTD_outBuffer zstd_output = {buffer_, buffer_size_, 0};

        size_t const ret = ZSTD_decompressStream(zstd_stream_, &zstd_output, &zstd_input_);

        if (ZSTD_isError(ret)) {
            return traits_type::eof();
        }

        uncompressed_bytes = zstd_output.pos;

        //        if (0 == uncompressed_bytes && 0 == ret && zstd_output.pos < zstd_output.size) {
        //            return traits_type::eof();
        //        }
    }

    char_type* current = buffer_;

    total_out_ += pos_type(uncompressed_bytes);

    setg(buffer_, current, current + uncompressed_bytes);

    return traits_type::to_int_type(*current);
}

Filebuffer::pos_type Filebuffer::seekpos(pos_type pos, std::ios_base::openmode) {
    if (!is_open() /*|| m_is_write_stream*/) {
        return pos_type(off_type(-1));
    }

    off_type const buffer_range  = egptr() - eback();
    pos_type const buffer_start  = total_out_ - buffer_range;
    off_type const buffer_offset = pos - buffer_start;

    if (buffer_offset >= 0 && buffer_offset < buffer_range) {
        // the new position is still in our current buffer
        setg(eback(), eback() + buffer_offset, egptr());
    } else {
        if (buffer_offset < 0) {
            // start everything from scratch
            stream_->seekg(data_start_);

            restart_zstd_stream();
        }

        for (pos_type const len = pos; total_out_ < len;) {
            underflow();
        }

        pos_type const difference = total_out_ - pos;

        setg(eback(), egptr() - difference, egptr());
    }

    return pos;
}

Filebuffer::pos_type Filebuffer::seekoff(off_type off, std::ios_base::seekdir dir,
                                         std::ios_base::openmode mode) {
    if (!is_open() /*|| m_is_write_stream*/) {
        return pos_type(off_type(-1));
    }

    off_type pos = off;

    switch (dir) {
        case std::ios_base::beg:
            break;

        case std::ios_base::cur: {
            //			PHYSFS_sint64 ptell = PHYSFS_tell(m_file);
            //			pos_type buf_tell = static_cast<pos_type>(ptell) -
            // static_cast<pos_type>(__streambuf_type::egptr() - __streambuf_type::gptr());

            //			if (!off) {
            //				return buf_tell;
            //			}

            //			pos += static_cast<off_type> (buf_tell);
            break;
        }

        case std::ios_base::end:
            //	pos = static_cast<off_type> (PHYSFS_fileLength(m_file)) - pos;
            break;

        default:
            //_ASSERTE(!"invalid seekdirection");
            return pos_type(off_type(-1));
    }

    return seekpos(static_cast<pos_type>(pos), mode);
}

// Write stuff:
Filebuffer::int_type Filebuffer::overflow(int_type /*c*/) {
    //	if (!is_open() || /*!m_is_write_stream*/)
    //	{
    //		return traits_type::eof();
    //	}

    //	size_t size = __streambuf_type::pptr() - __streambuf_type::pbase();

    //	if (!size)
    //	{
    //		return 0;
    //	}

    //	PHYSFS_sint64 res = PHYSFS_write(m_file, __streambuf_type::pbase(),
    // PHYSFS_uint32(sizeof(char_type)), size);

    //	if (res <= 0)
    //	{
    //		return traits_type::eof();
    //	}

    //	if (!traits_type::eq_int_type(c, traits_type::eof()))
    //	{
    //		PHYSFS_sint64 res = PHYSFS_write(m_file, &c, sizeof(char_type), 1);

    //		if (res <= 0)
    //		{
    //			return traits_type::eof();
    //		}
    //	}

    //	char_type* xend = (static_cast<size_t> (res) == buffer_.size()) ? &*buffer_.end() :
    //&buffer_[res]; 	setp(&*buffer_.begin(), xend);

    return 0;
}

int Filebuffer::sync() {
    if (!is_open() /* || !m_is_write_stream*/) {
        return -1;
    }

    return overflow(traits_type::eof());
}

std::streamsize Filebuffer::showmanyc() {
    if (!is_open() /*|| m_is_write_stream*/) {
        return -1;
    }

    //	PHYSFS_sint64 fileSize = PHYSFS_fileLength(m_file);

    //	return int32_t(fileSize);

    return -1;
}

void Filebuffer::restart_zstd_stream() {
    zstd_input_ = {read_buffer_, 0, 0};

    total_out_ = 0;
}

}  // namespace file::zstd
