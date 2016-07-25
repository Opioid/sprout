#pragma once

#include "miniz/miniz.hpp"
#include <streambuf>
#include <istream>
#include <array>

namespace gzip {

class Filebuffer : public std::basic_streambuf<char, std::char_traits<char>> {

public:

	using char_type = char ;
	using traits_type = std::char_traits<char>;
	using int_type = traits_type::int_type;
	using pos_type = traits_type::pos_type;
	using off_type = traits_type::off_type;

	using __streambuf_type = std::basic_streambuf<char, std::char_traits<char>>;

	Filebuffer();

	virtual ~Filebuffer();

	bool is_open() const;

	Filebuffer* open(const char* filename, std::ios_base::openmode mode);

	Filebuffer* open(std::istream* stream);

	Filebuffer* close();

protected:

	virtual int_type underflow() final override;

	virtual pos_type seekpos(pos_type pos, std::ios_base::openmode) final override;

	virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir,
							 std::ios_base::openmode mode) final override;

	virtual int_type overflow(int_type c = traits_type::eof()) final override;

	virtual int sync() final override;

	virtual std::streamsize showmanyc() final override;

private:

	bool init_z_stream();

	pos_type data_start_;

	std::istream* stream_;

	mz_stream z_stream_;

	std::array<char_type, 8192> read_buffer_;

	std::array<char_type, 8192> buffer_;
};

class Read_stream : public std::basic_istream<char, std::char_traits<char>> {

public:

	using char_type = char;
	using traits_type = std::char_traits<char>;
	using int_type = traits_type::int_type;
	using pos_type = traits_type::pos_type;
	using off_type = traits_type::off_type;

	using __istream_type = std::basic_istream<char, std::char_traits<char>>;

	Read_stream();

	explicit Read_stream(const std::string& name,
						 std::ios_base::openmode mode = std::ios_base::binary);

	explicit Read_stream(const char* name,
						 std::ios_base::openmode mode = std::ios_base::binary);

	explicit Read_stream(std::istream* stream);

	const Filebuffer* rdbuf() const;

	Filebuffer* rdbuf();

	bool is_open() const;

	void open(char const* name, 
			  std::ios_base::openmode mode = std::ios_base::binary);
	
	void open(std::istream* stream);

	void close();

private:

	Filebuffer stream_buffer_;
};

}
