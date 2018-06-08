#ifndef SU_BASE_MEMORY_TILED_ARRAY_HPP
#define SU_BASE_MEMORY_TILED_ARRAY_HPP

#include <cstdint>

namespace memory {

template <typename T, uint32_t Log_tile_size>
class Tiled_array {
  public:
    Tiled_array(uint32_t dimensions_x, uint32_t dimensions_y);

    T& at(uint32_t i);

    T& serialized_at(uint32_t i);

    T const& at(uint32_t x, uint32_t y) const;
    T&       at(uint32_t x, uint32_t y);

    void const* data() const;

  private:
    constexpr uint32_t tile_size() const;

    uint32_t round_up(uint32_t x) const;

    uint32_t block(uint32_t x) const;
    uint32_t offset(uint32_t x) const;

    T* data_;

    uint32_t dimensions_x_;
    uint32_t dimensions_y_;
    uint32_t num_tiles_x_;
};

}  // namespace memory

#endif
