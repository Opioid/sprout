#include "gzip_read_stream.hpp"
#include <fstream>
#include <limits>
#include <string>

namespace gzip {

Filebuffer::Filebuffer() {}

Filebuffer::~Filebuffer() {
	close();
}

bool Filebuffer::is_open() const {
	return nullptr != stream_;
}

Filebuffer* Filebuffer::open(const char* filename, std::ios_base::openmode mode) {
	// Return failure if this file buf is open already
	if (is_open()) {
		return nullptr;
	}

	auto stream = std::unique_ptr<std::istream>(new std::ifstream(filename, mode));

	open(std::move(stream));

	return this;
}

Filebuffer* Filebuffer::open(std::unique_ptr<std::istream> stream) {
	stream_ = std::move(stream);

	uint8_t header[10];
	stream_->read(reinterpret_cast<char*>(header), sizeof(header));

	if (header[0] != 0x1F || header[1] != 0x8B) {
		return nullptr;
	}

	if (header[2] != 8) {
		// unknown compression algorithm
		return nullptr;
	}

	if (header[3] & 1 << 2) {
		// FEXTRA
		uint8_t n[2];
		stream_->read(reinterpret_cast<char*>(n), sizeof(n));
		size_t len = n[0] << 0 | n[1] << 8;
		stream_->ignore(len);
	}

	if (header[3] & 1 << 3) {
		// FNAME
		stream_->ignore(std::numeric_limits<std::streamsize>::max(), char(0));
	}

	if (header[3] & 1 << 4) {
		// FCOMMENT
		stream_->ignore(std::numeric_limits<std::streamsize>::max(), char(0));
	}

	if (header[3] & 1 << 1) {
		// FCRC
		stream_->ignore(2);
	}

	data_start_ = stream_->tellg();

	if (!init_z_stream()) {
		return nullptr;
	}

	return this;
}

Filebuffer* Filebuffer::close() {
	// Return failure if this file buf is closed already
	if (!is_open()) {
		return nullptr;
	}

	sync();

	mz_inflateEnd(&z_stream_);

	stream_.release();

	return this;
}

Filebuffer::int_type Filebuffer::underflow() {
	if (!is_open()) {
		return traits_type::eof();
	}

	char_type* current = gptr();

	size_t uncompressed_bytes = 0;

	while (0 == uncompressed_bytes) {
		if (0 == z_stream_.avail_in) {
			stream_->read(read_buffer_.data(), read_buffer_.size());

			size_t read_bytes = stream_ ? read_buffer_.size() : stream_->gcount();

			z_stream_.avail_in = static_cast<uint32_t>(read_bytes);
			z_stream_.next_in  = reinterpret_cast<uint8_t*>(read_buffer_.data());
		}

		if (0 == z_stream_.avail_out) {
			z_stream_.avail_out = static_cast<uint32_t>(buffer_.size());
			z_stream_.next_out  = reinterpret_cast<uint8_t*>(buffer_.data());

			current = &*buffer_.begin();
		}

		uint32_t avail_out = z_stream_.avail_out;

		int status = mz_inflate(&z_stream_, MZ_NO_FLUSH);
		if (status != MZ_OK && status != MZ_STREAM_END
		&&  status != MZ_BUF_ERROR && status != MZ_NEED_DICT) {
			return traits_type::eof();
		}

		uncompressed_bytes = avail_out - z_stream_.avail_out;

		if (0 == uncompressed_bytes && MZ_STREAM_END == status) {
			return traits_type::eof();
		}
	}

	setg(&*buffer_.begin(), current, current + uncompressed_bytes);

	return traits_type::to_int_type(*current);
}

Filebuffer::pos_type Filebuffer::seekpos(pos_type pos, std::ios_base::openmode) {
	if (!is_open() /*|| m_is_write_stream*/) {
		return pos_type(off_type(-1));
	}

	off_type buffer_range = egptr() - eback();
	pos_type buffer_start = z_stream_.total_out - buffer_range;
	off_type buffer_offset = pos - buffer_start;

	if (buffer_offset >= 0 && buffer_offset < buffer_range) {
		// the new position is still in our current buffer
		setg(eback(), eback() + buffer_offset, egptr());
	} else if (buffer_offset < 0) {
		// start everything from scratch

		stream_->seekg(data_start_);

		mz_inflateEnd(&z_stream_);

		init_z_stream();

		setg(&*buffer_.begin(), &*buffer_.begin(), &*buffer_.begin());
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
//			pos_type buf_tell = static_cast<pos_type>(ptell) - static_cast<pos_type>(__streambuf_type::egptr() - __streambuf_type::gptr());

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

//	PHYSFS_sint64 res = PHYSFS_write(m_file, __streambuf_type::pbase(), PHYSFS_uint32(sizeof(char_type)), size);

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

//	char_type* xend = (static_cast<size_t> (res) == buffer_.size()) ? &*buffer_.end() : &buffer_[res];
//	setp(&*buffer_.begin(), xend);

	return 0;
}

int Filebuffer::sync() {
	if (!is_open()/* || !m_is_write_stream*/) {
		return -1;
	}

	return overflow(traits_type::eof());
}

std::streamsize Filebuffer::showmanyc() {
	if (!is_open() /*|| m_is_write_stream*/) {
		return -1;
	}

//	PHYSFS_sint64 fileSize = PHYSFS_fileLength(m_file);

//	return static_cast<int>(fileSize);

	return -1;
}

bool Filebuffer::init_z_stream() {
	z_stream_.zalloc = nullptr;
	z_stream_.zfree  = nullptr;

	if (MZ_OK != mz_inflateInit2(&z_stream_, -MZ_DEFAULT_WINDOW_BITS)) {
		return false;
	}

	z_stream_.avail_in  = 0;
	z_stream_.avail_out = 0;

	return true;
}

Read_stream::Read_stream() : __istream_type(&stream_buffer_) {}

Read_stream::Read_stream(const std::string& name, std::ios_base::openmode mode) :
	__istream_type(&stream_buffer_)/*, name_(name)*/ {
	open(name.c_str(), mode);
}

Read_stream::Read_stream(const char* name, std::ios_base::openmode mode) :
	__istream_type(&stream_buffer_)/*, name_(name)*/ {
	open(name, mode);
}

Read_stream::Read_stream(std::unique_ptr<std::istream> stream) : __istream_type(&stream_buffer_) {
	open(std::move(stream));
}

const Filebuffer* Read_stream::rdbuf() const {
	return &stream_buffer_;
}

Filebuffer* Read_stream::rdbuf() {
	return &stream_buffer_;
}

bool Read_stream::is_open() const {
	return rdbuf()->is_open();
}

void Read_stream::open(const char* name, std::ios_base::openmode mode) {
	if (!rdbuf()->open(name, mode | std::ios_base::in)) {
		__istream_type::setstate(std::ios_base::failbit);
	}
}

void Read_stream::open(std::unique_ptr<std::istream> stream) {
	if (!rdbuf()->open(std::move(stream))) {
		__istream_type::setstate(std::ios_base::failbit);
	}
}

void Read_stream::close() {
	if (!rdbuf()->close()) {
		__istream_type::setstate(std::ios_base::failbit);
	}
}

}
