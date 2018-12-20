#ifndef SU_EXTENSION_PROCEDURAL_FLUID_GRID_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_GRID_HPP

#include "base/math/matrix3x3.hpp"

namespace procedural::fluid {

template <typename T>
class Grid {
  public:
    Grid(int3 const& dimensions) noexcept;

    ~Grid() noexcept;

    int3 const& dimensions() const noexcept;

    T const& at(int32_t index) const noexcept;

    void clear(T const& value) noexcept;

    void set(int3 const& c, T const& value) noexcept;

    T interpolate(float3 const& position) noexcept;

    T* data() noexcept;

  private:
    int3 dimensions_;

    T* voxels_;
};

extern template class Grid<float3>;
extern template class Grid<float3x3>;

}  // namespace procedural::fluid

#endif
