#pragma once

#include "rapidjson/rapidjson.h"
#include <istream>

//! File byte stream for input using fread().
/*!
    \implements Stream
*/
namespace json {

class Read_stream {
public:

	typedef char Ch;	//!< Character type (byte).

	Read_stream(std::istream& stream, size_t buffer_size = 4096) :
		stream_(&stream),  m_buffer_size(buffer_size), buffer_(new Ch[buffer_size]),
		m_buffer_last(0), m_current(buffer_), m_count(0),  m_owns_buffer(false) {
        RAPIDJSON_ASSERT(stream_ != nullptr);
        RAPIDJSON_ASSERT(buffer_size >= 4);
        Read();
    }

	Read_stream(std::istream& stream, size_t buffer_size, Ch* buffer) :
		stream_(&stream),  m_buffer_size(buffer_size), buffer_(buffer),
		m_buffer_last(0), m_current(buffer_), m_count(0), m_owns_buffer(false) {
        RAPIDJSON_ASSERT(stream_ != nullptr);
        RAPIDJSON_ASSERT(buffer_size >= 4);
        Read();
    }

	~Read_stream() {
		if (m_owns_buffer) {
            delete [] buffer_;
        }
    }

	Ch Peek() const {
        return *m_current;
    }

	Ch Take() {
		Ch c = *m_current;
        Read();
        return c;
    }

	size_t Tell() const {
        return m_count + (m_current - buffer_);
    }

    // Not implemented
	void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); }
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return nullptr; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

    // For encoding detection only.
	const Ch* Peek4() const {
        return (m_current + 4 <= m_buffer_last) ? m_current : nullptr;
    }

private:

	void Read() {
		if (m_current < m_buffer_last) {
            ++m_current;
		} else if (*stream_) {
            m_count += stream_->gcount();

            stream_->read(buffer_, m_buffer_size);

            m_buffer_last = buffer_ + stream_->gcount() - 1;
            m_current = buffer_;

			if (static_cast<size_t>(stream_->gcount()) < m_buffer_size) {
                buffer_[stream_->gcount()] = '\0';
                ++m_buffer_last;
            }
        }
    }

    std::istream* stream_;
    size_t m_buffer_size;
	Ch* buffer_;
	Ch* m_buffer_last;
	Ch* m_current;
    size_t m_count;	//!< Number of characters read

    bool m_owns_buffer;
};

}

