#ifndef SU_BASE_MEMORY_TILED_ARRAY_INL
#define SU_BASE_MEMORY_TILED_ARRAY_INL

#include "tiled_array.hpp"

namespace memory {

template<typename T, uint32_t Log_tile_size>
Tiled_array<T, Log_tile_size>::Tiled_array(uint32_t dimensions_x, uint32_t dimensions_y) :
	data_(new T[round_up(dimensions_x * dimensions_y)]),
	dimensions_x_(dimensions_x), dimensions_y_(dimensions_y),
	num_tiles_x_(round_up(dimensions_x) >> Log_tile_size)
{}

template<typename T, uint32_t Log_tile_size>
T& Tiled_array<T, Log_tile_size>::at(uint32_t i) {
	return data_[i];
}

template<typename T, uint32_t Log_tile_size>
T& Tiled_array<T, Log_tile_size>::serialized_at(uint32_t i) {
	uint32_t x = i % dimensions_x_;
	uint32_t y = i / dimensions_x_;
	return at(x, y);
}

template<typename T, uint32_t Log_tile_size>
T const& Tiled_array<T, Log_tile_size>::at(uint32_t x, uint32_t y) const {
	uint32_t bx = block(x);
	uint32_t by = block(y);
	uint32_t ox = offset(x);
	uint32_t oy = offset(y);

	uint32_t index = (tile_size() * tile_size() * (num_tiles_x_ * by + bx)) + tile_size() * oy + ox;

	return data_[index];
}


template<typename T, uint32_t Log_tile_size>
T& Tiled_array<T, Log_tile_size>::at(uint32_t x, uint32_t y) {
	uint32_t bx = block(x);
	uint32_t by = block(y);
	uint32_t ox = offset(x);
	uint32_t oy = offset(y);

	uint32_t index = (tile_size() * tile_size() * (num_tiles_x_ * by + bx)) + tile_size() * oy + ox;

	return data_[index];
}

template<typename T, uint32_t Log_tile_size>
void const* Tiled_array<T, Log_tile_size>::data() const {
	return static_cast<void*>(data_);
}

template<typename T, uint32_t Log_tile_size>
constexpr uint32_t Tiled_array<T, Log_tile_size>::tile_size() const {
	return 1 << Log_tile_size;
}

template<typename T, uint32_t Log_tile_size>
uint32_t Tiled_array<T, Log_tile_size>::round_up(uint32_t x) const {
	return (x + tile_size() - 1) & ~(tile_size() - 1);
}

template<typename T, uint32_t Log_tile_size>
uint32_t Tiled_array<T, Log_tile_size>::block(uint32_t x) const {
	return x >> Log_tile_size;
}

template<typename T, uint32_t Log_tile_size>
uint32_t Tiled_array<T, Log_tile_size>::offset(uint32_t x) const {
	return x & (tile_size() - 1);
}

}

#endif
