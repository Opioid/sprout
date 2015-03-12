#pragma once

namespace rapidjson
{

class CrtAllocator;

template <typename BaseAllocator> class MemoryPoolAllocator;

template <typename Encoding, typename Allocator> class GenericValue;

template<typename CharType> struct UTF8;

typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator>> Value;

}
