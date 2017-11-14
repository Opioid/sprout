#ifndef SU_BASE_JSON_TYPES_HPP
#define SU_BASE_JSON_TYPES_HPP

namespace rapidjson {

class CrtAllocator;

template <typename BaseAllocator> class MemoryPoolAllocator;

template <typename Encoding, typename Allocator> class GenericValue;

template<typename CharType> struct UTF8;

using Value = GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator>>;

}

namespace json {

using Value = rapidjson::Value;

}

#endif
