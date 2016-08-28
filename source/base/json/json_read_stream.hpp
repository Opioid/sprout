#pragma once

#include "rapidjson/rapidjson.h"
#include <istream>

//! File byte stream for input using fread().
/*!
    \implements Stream
*/
namespace json {

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(padded)
#endif

#ifdef _MSC_VER
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4351) // new behavior: elements of array 'array' will be default initialized
#endif

//! Wrapper of \c std::basic_istream into RapidJSON's Stream concept.
/*!
	The classes can be wrapped including but not limited to:
	- \c std::istringstream
	- \c std::stringstream
	- \c std::wistringstream
	- \c std::wstringstream
	- \c std::ifstream
	- \c std::fstream
	- \c std::wifstream
	- \c std::wfstream
	\tparam StreamType Class derived from \c std::basic_istream.
*/

template <typename StreamType>
class BasicIStreamWrapper {
public:
	typedef typename StreamType::char_type Ch;
	BasicIStreamWrapper(StreamType& stream) : stream_(stream), count_(), peekBuffer_() {}

	Ch Peek() const {
		typename StreamType::int_type c = stream_.peek();
		return c != StreamType::traits_type::eof() ? static_cast<Ch>(c) : '\0';
	}

	Ch Take() {
		typename StreamType::int_type c = stream_.get();
		if (c != StreamType::traits_type::eof()) {
			count_++;
			return static_cast<Ch>(c);
		}
		else
			return '\0';
	}

	// tellg() may return -1 when failed. So we count by ourself.
	size_t Tell() const { return count_; }

	Ch* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	void Put(Ch) { RAPIDJSON_ASSERT(false); }
	void Flush() { RAPIDJSON_ASSERT(false); }
	size_t PutEnd(Ch*) { RAPIDJSON_ASSERT(false); return 0; }

	// For encoding detection only.
	const Ch* Peek4() const {
		RAPIDJSON_ASSERT(sizeof(Ch) == 1); // Only usable for byte stream.
		int i;
		bool hasError = false;
		for (i = 0; i < 4; ++i) {
			typename StreamType::int_type c = stream_.get();
			if (c == StreamType::traits_type::eof()) {
				hasError = true;
				stream_.clear();
				break;
			}
			peekBuffer_[i] = static_cast<Ch>(c);
		}
		for (--i; i >= 0; --i)
			stream_.putback(peekBuffer_[i]);
		return !hasError ? peekBuffer_ : 0;
	}

private:
	BasicIStreamWrapper(const BasicIStreamWrapper&);
	BasicIStreamWrapper& operator=(const BasicIStreamWrapper&);

	StreamType& stream_;
	size_t count_;  //!< Number of characters read. Note:
	mutable Ch peekBuffer_[4];
};

typedef BasicIStreamWrapper<std::istream> IStreamWrapper;
typedef BasicIStreamWrapper<std::wistream> WIStreamWrapper;

#if defined(__clang__) || defined(_MSC_VER)
RAPIDJSON_DIAG_POP
#endif

}

