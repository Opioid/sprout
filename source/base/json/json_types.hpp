#ifndef SU_BASE_JSON_TYPES_HPP
#define SU_BASE_JSON_TYPES_HPP

namespace rapidjson {

template<typename CharType> struct UTF8;

class CrtAllocator;

template <typename BaseAllocator> class MemoryPoolAllocator;

template <typename Encoding, typename Allocator, typename StackAllocator> class GenericDocument;

using Document = GenericDocument<UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator>;

template <typename Encoding, typename Allocator> class GenericValue;

using Value = GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator>>;

}

namespace json {

using Document = rapidjson::Document;
using Value = rapidjson::Value;

}

#endif
