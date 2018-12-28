#ifndef SU_EXTENSION_PROCEDURAL_FLUID_GRID_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_GRID_HPP

#include "base/math/matrix3x3.hpp"
#include <vector>

namespace procedural::fluid {

template <typename T>
class Grid {
  public:
    Grid(int3 const& dimensions) noexcept;

    ~Grid() noexcept;

    int3 const& dimensions() const noexcept;

    T const& at(int32_t index) const noexcept;

	T& at(float3 const& v) noexcept;

    void clear(T const& value) noexcept;

    void set(int3 const& c, T const& value) noexcept;

    T interpolate(float3 const& uvw) noexcept;

    T* data() noexcept;

  private:
    float3 map(float3 const& uvw, int3& xyz0, int3& xyz1) const noexcept;

	T const& at(int32_t x, int32_t y, int32_t z) const noexcept;

    int3 dimensions_;

    T* voxels_;
};

extern template class Grid<float3>;
extern template class Grid<float3x3>;
extern template class Grid<std::vector<uint32_t>>;

}  // namespace procedural::fluid

#endif
