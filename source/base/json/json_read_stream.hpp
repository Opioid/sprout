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

	using Ch = char;	//!< Character type (byte).

	Read_stream(std::istream& stream, size_t buffer_size = 4096) :
		stream_(&stream),  buffer_size_(buffer_size), buffer_(new Ch[buffer_size]),
		buffer_last_(0), current_(buffer_), count_(0),  owns_buffer_(false) {
        RAPIDJSON_ASSERT(stream_ != nullptr);
        RAPIDJSON_ASSERT(buffer_size >= 4);
        Read();
    }

	Read_stream(std::istream& stream, size_t buffer_size, Ch* buffer) :
		stream_(&stream),  buffer_size_(buffer_size), buffer_(buffer),
		buffer_last_(0), current_(buffer_), count_(0), owns_buffer_(false) {
        RAPIDJSON_ASSERT(stream_ != nullptr);
        RAPIDJSON_ASSERT(buffer_size >= 4);
        Read();
    }

	~Read_stream() {
		if (owns_buffer_) {
            delete [] buffer_;
        }
    }

	Ch Peek() const {
		return *current_;
    }

	Ch Take() {
		Ch c = *current_;
        Read();
        return c;
    }

	size_t Tell() const {
		return count_ + (current_ - buffer_);
    }

    // Not implemented
	void Put(Ch) { RAPIDJSON_ASSERT(false); }
    void Flush() { RAPIDJSON_ASSERT(false); }
	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return nullptr; }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

    // For encoding detection only.
	const Ch* Peek4() const {
		return (current_ + 4 <= buffer_last_) ? current_ : nullptr;
    }

private:

	void Read() {
		if (current_ < buffer_last_) {
			++current_;
		} else if (*stream_) {
			count_ += stream_->gcount();

			stream_->read(buffer_, buffer_size_);

			buffer_last_ = buffer_ + stream_->gcount() - 1;
			current_ = buffer_;

			if (static_cast<size_t>(stream_->gcount()) < buffer_size_) {
                buffer_[stream_->gcount()] = '\0';
				++buffer_last_;
            }
        }
    }

    std::istream* stream_;
	size_t buffer_size_;
	Ch* buffer_;
	Ch* buffer_last_;
	Ch* current_;
	size_t count_;	//!< Number of characters read

	bool owns_buffer_;
};

}

