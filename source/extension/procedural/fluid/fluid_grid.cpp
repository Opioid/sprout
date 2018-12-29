#include "fluid_grid.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "core/image/texture/sampler/address_mode.hpp"
#include "core/image/texture/sampler/bilinear.hpp"

namespace procedural::fluid {

template <typename T>
Grid<T>::Grid(int3 const& dimensions) noexcept
    : dimensions_(dimensions),
      //   voxels_(memory::allocate_aligned<T>(dimensions[0] * dimensions[1] * dimensions[2])),
      voxels_(new T[dimensions[0] * dimensions[1] * dimensions[2]])

{}

template <typename T>
Grid<T>::~Grid() noexcept {
    //    memory::free_aligned(voxels_);
    delete[] voxels_;
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
T& Grid<T>::at(float3 const& v) noexcept {
    int3 const c = int3(v * float3(dimensions_));

    int32_t const i = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    return voxels_[i];
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
T Grid<T>::interpolate(float3 const& uvw) noexcept {
    /*
    int3 c(uvw * float3(dimensions_));

    c = min(max(c, int3(0)), dimensions_ - 1);

    int32_t const i = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    return voxels_[i];
        */

    using namespace image::texture::sampler;

    int3         xyz, xyz1;
    float3 const stu = map(uvw, xyz, xyz1);

    T const c000 = at(xyz[0], xyz[1], xyz[2]);
    T const c100 = at(xyz1[0], xyz[1], xyz[2]);
    T const c010 = at(xyz[0], xyz1[1], xyz[2]);
    T const c110 = at(xyz1[0], xyz1[1], xyz[2]);
    T const c001 = at(xyz[0], xyz[1], xyz1[2]);
    T const c101 = at(xyz1[0], xyz[1], xyz1[2]);
    T const c011 = at(xyz[0], xyz1[1], xyz1[2]);
    T const c111 = at(xyz1[0], xyz1[1], xyz1[2]);

    T const c0 = bilinear(c000, c100, c010, c110, stu[0], stu[1]);
    T const c1 = bilinear(c001, c101, c011, c111, stu[0], stu[1]);

    return lerp(c0, c1, stu[2]);
}

template <>
std::vector<uint32_t> Grid<std::vector<uint32_t>>::interpolate(float3 const& uvw) noexcept {
    int3 c(uvw * float3(dimensions_));

    c = min(max(c, int3(0)), dimensions_ - 1);

    int32_t const i = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    return voxels_[i];
}

template <typename T>
T* Grid<T>::data() noexcept {
    return voxels_;
}

template <typename T>
float3 Grid<T>::map(float3 const& uvw, int3& xyz0, int3& xyz1) const noexcept {
    using namespace image::texture::sampler;

    float3 const d = float3(dimensions_);

    float const u = Address_mode_clamp::f(uvw[0]) * d[0] - 0.5f;
    float const v = Address_mode_clamp::f(uvw[1]) * d[1] - 0.5f;
    float const w = Address_mode_clamp::f(uvw[2]) * d[2] - 0.5f;

    float const fu = std::floor(u);
    float const fv = std::floor(v);
    float const fw = std::floor(w);

    int32_t const x = static_cast<int32_t>(fu);
    int32_t const y = static_cast<int32_t>(fv);
    int32_t const z = static_cast<int32_t>(fw);

    int3 const b = dimensions_ - 1;

    xyz0[0] = Address_mode_clamp::lower_bound(x, b[0]);
    xyz0[1] = Address_mode_clamp::lower_bound(y, b[1]);
    xyz0[2] = Address_mode_clamp::lower_bound(z, b[2]);

    xyz1[0] = Address_mode_clamp::increment(x, b[0]);
    xyz1[1] = Address_mode_clamp::increment(y, b[1]);
    xyz1[2] = Address_mode_clamp::increment(z, b[2]);

    return float3(u - fu, v - fv, w - fw);
}

template <typename T>
T const& Grid<T>::at(int32_t x, int32_t y, int32_t z) const noexcept {
    int32_t const i = (z * dimensions_[1] + y) * dimensions_[0] + x;

    return voxels_[i];
}

template class Grid<float3>;
template class Grid<float3x3>;
template class Grid<std::vector<uint32_t>>;

}  // namespace procedural::fluid
