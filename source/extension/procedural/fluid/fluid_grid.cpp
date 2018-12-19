#include "fluid_grid.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

namespace procedural::fluid {

template <typename T>
Grid<T>::Grid(int3 const& dimensions) noexcept
    : dimensions_(dimensions),
      voxels_(memory::allocate_aligned<T>(dimensions[0] * dimensions[1] * dimensions[2])) {}

template <typename T>
Grid<T>::~Grid() noexcept {
    memory::free_aligned(voxels_);
}

template <typename T>
int3 const& Grid<T>::dimensions() const noexcept {
    return dimensions_;
}

template <typename T>
T const& Grid<T>::at(int32_t index) const noexcept {
    return voxels_[index];
}

template <typename T>
void Grid<T>::clear(T const& value) noexcept {
    int32_t const len = dimensions_[0] * dimensions_[1] * dimensions_[2];

    for (int32_t i = 0; i < len; ++i) {
        voxels_[i] = value;
    }
}

template <typename T>
void Grid<T>::set(int3 const& c, T const& value) noexcept {
    int32_t const i = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    voxels_[i] = value;
}

template <typename T>
T Grid<T>::interpolate(float3 const& position) noexcept {
    const int3 c(position);

    int32_t const i = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    return voxels_[i];
}

template <typename T>
T* Grid<T>::data() noexcept {
    return voxels_;
}

template class Grid<float3>;
template class Grid<float3x3>;

}  // namespace procedural::fluid
