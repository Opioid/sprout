#include "typed_image.hpp"
#include "base/math/vector4.inl"

#include <cstring>

namespace image {

Description::Description() : dimensions_(0), offset_(0) {}

Description::Description(int2 dimensions) : dimensions_(dimensions, 1), offset_(0) {}

Description::Description(int3_p dimensions, int3_p offset)
    : dimensions_(dimensions), offset_(offset) {}

uint64_t Description::num_pixels() const {
    return uint64_t(dimensions_[0]) * uint64_t(dimensions_[1]) * uint64_t(dimensions_[2]);
}

int3 Description::dimensions() const {
    return dimensions_;
}

int3 Description::offset() const {
    return offset_;
}

template <typename T>
Typed_image<T>::Typed_image() : data_(nullptr) {}

template <typename T>
Typed_image<T>::Typed_image(Description const& description)
    : description_(description), data_(new T[description.num_pixels()]) {}

template <typename T>
Typed_image<T>::Typed_image(Typed_image&& other) noexcept
    : description_(other.description_), data_(other.data_) {
    other.data_ = nullptr;
}

template <typename T>
Typed_image<T>::~Typed_image() {
    delete[] data_;
}

template <typename T>
Description const& Typed_image<T>::description() const {
    return description_;
}

template <typename T>
int2 Typed_image<T>::coordinates_2(int32_t index) const {
    int2 c;
    c[1] = index / description_.dimensions_[0];
    c[0] = index - c[1] * description_.dimensions_[0];
    return c;
}

template <typename T>
void Typed_image<T>::resize(Description const& description) {
    if (description.num_pixels() == description_.num_pixels()) {
        return;
    }

    delete[] data_;

    description_.dimensions_ = description.dimensions_;

    data_ = new T[description_.num_pixels()];
}

template <typename T>
void Typed_image<T>::clear(T v) {
    for (int64_t i = 0, len = description_.num_pixels(); i < len; ++i) {
        data_[i] = v;
    }
}

template <typename T>
void Typed_image<T>::store(int32_t index, T v) {
    data_[index] = v;
}

template <typename T>
T Typed_image<T>::at(int32_t index) const {
    return data_[index];
}

template <typename T>
void Typed_image<T>::store(int32_t x, int32_t y, T v) {
    int32_t const i = y * description_.dimensions_[0] + x;
    data_[i]        = v;
}

template <typename T>
T Typed_image<T>::at(int32_t x, int32_t y) const {
    int32_t const i = y * description_.dimensions_[0] + x;
    return data_[i];
}

template <typename T>
void Typed_image<T>::gather(int4_p xy_xy1, T c[4]) const {
    int32_t const width = description_.dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = data_[y0 + xy_xy1[0]];
    c[1] = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    c[2] = data_[y1 + xy_xy1[0]];
    c[3] = data_[y1 + xy_xy1[2]];
}

template <typename T>
T Typed_image<T>::at(int32_t x, int32_t y, int32_t z) const {
    int2 const d = description_.dimensions_.xy();

    int64_t const i = (int64_t(z) * int64_t(d[1]) + int64_t(y)) * int64_t(d[0]) + int64_t(x);
    return data_[i];
}

template <typename T>
void Typed_image<T>::gather(int3_p xyz, int3_p xyz1, T c[8]) const {
    int64_t const w = int64_t(description_.dimensions_[0]);
    int64_t const h = int64_t(description_.dimensions_[1]);

    int64_t const x = int64_t(xyz[0]);
    int64_t const y = int64_t(xyz[1]);
    int64_t const z = int64_t(xyz[2]);

    int64_t const x1 = int64_t(xyz1[0]);
    int64_t const y1 = int64_t(xyz1[1]);
    int64_t const z1 = int64_t(xyz1[2]);

    int64_t const d = z * h;

    int64_t const c0 = (d + y) * w + x;
    c[0]             = data_[c0];

    int64_t const c1 = (d + y) * w + x1;
    c[1]             = data_[c1];

    int64_t const c2 = (d + y1) * w + x;
    c[2]             = data_[c2];

    int64_t const c3 = (d + y1) * w + x1;
    c[3]             = data_[c3];

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    c[4]             = data_[c4];

    int64_t const c5 = (d1 + y) * w + x1;
    c[5]             = data_[c5];

    int64_t const c6 = (d1 + y1) * w + x;
    c[6]             = data_[c6];

    int64_t const c7 = (d1 + y1) * w + x1;
    c[7]             = data_[c7];
}

template <typename T>
T* Typed_image<T>::data() const {
    return data_;
}

template <typename T>
void Typed_image<T>::copy(Typed_image& destination) const {
    std::copy(&data_[0], &data_[description_.num_pixels() - 1], destination.data_);
}

// template <typename T>
// Typed_sparse_image<T>::Typed_sparse_image()  = default;

template <typename T>
Typed_sparse_image<T>::Typed_sparse_image(Description const& description)
    : description_(description) {
    int3 const d = description.dimensions_;

    num_cells_ = d >> Log2_cell_dim;

    num_cells_ += math::min(d - (num_cells_ << Log2_cell_dim), 1);

    int32_t const cells_len = num_cells_[0] * num_cells_[1] * num_cells_[2];

    cells_ = new Cell[uint32_t(cells_len)];

    for (int32_t i = 0; i < cells_len; ++i) {
        cells_[i].data  = nullptr;
        cells_[i].value = T(0);
    }
}

template <typename T>
Typed_sparse_image<T>::Typed_sparse_image(Typed_sparse_image&& other) noexcept
    : description_(other.description()), num_cells_(other.num_cells_), cells_(other.cells_) {
    other.num_cells_ = int3(0);
    other.cells_     = nullptr;
}

template <typename T>
Typed_sparse_image<T>::~Typed_sparse_image() {
    int32_t const cell_len = num_cells_[0] * num_cells_[1] * num_cells_[2];

    for (int32_t i = 0; i < cell_len; ++i) {
        delete[] cells_[i].data;
    }

    delete[] cells_;
}

template <typename T>
Description const& Typed_sparse_image<T>::description() const {
    return description_;
}

template <typename T>
void Typed_sparse_image<T>::store_sequentially(int64_t index, T v) {
    int3 const c  = coordinates_3(index);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    int32_t constexpr len = Cell_dim * Cell_dim * Cell_dim;

    Cell& cell = cells_[cell_index];

    if (!cell.data) {
        cell.data = new T[len];

        std::memset(cell.data, 0, len * sizeof(T));
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t const ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    cell.data[ci] = v;

    if (ci == len - 1) {
        bool homogeneous = true;
        T    value       = cell.data[0];

        for (int32_t i = 1; i < len; ++i) {
            if (value != cell.data[i]) {
                homogeneous = false;
                break;
            }
        }

        if (homogeneous) {
            delete[] cell.data;

            cell.data  = nullptr;
            cell.value = value;
        }
    }
}

template <typename T>
T Typed_sparse_image<T>::at(int64_t index) const {
    int3 const c  = coordinates_3(index);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    Cell const& cell = cells_[cell_index];

    if (!cell.data) {
        return cell.value;
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t const ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    return cell.data[ci];
}

template <typename T>
void Typed_sparse_image<T>::store(int32_t /*x*/, int32_t /*y*/, T /*v*/) {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    data_[i]        = v;
}

template <typename T>
T Typed_sparse_image<T>::at(int32_t /*x*/, int32_t /*y*/) const {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    return data_[i];

    return T(0);
}

template <typename T>
T Typed_sparse_image<T>::at(int32_t x, int32_t y, int32_t z) const {
    int3 const c(x, y, z);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    Cell const& cell = cells_[cell_index];

    if (!cell.data) {
        return cell.value;
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t const ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    return cell.data[ci];
}

template <typename T>
void Typed_sparse_image<T>::gather(int4_p /*xy_xy1*/, T c[4]) const {
    //    int32_t const width = description_.dimensions[0];

    //    int32_t const y0 = width * xy_xy1[1];

    //    c[0] = data_[y0 + xy_xy1[0]];
    //    c[1] = data_[y0 + xy_xy1[2]];

    //    int32_t const y1 = width * xy_xy1[3];

    //    c[2] = data_[y1 + xy_xy1[0]];
    //    c[3] = data_[y1 + xy_xy1[2]];

    c[0] = T(0);
    c[1] = T(0);
    c[2] = T(0);
    c[3] = T(0);
}

template <typename T>
void Typed_sparse_image<T>::gather(int3_p xyz, int3_p xyz1, T c[8]) const {
    int3 const cc0 = xyz >> Log2_cell_dim;
    int3 const cc1 = xyz1 >> Log2_cell_dim;

    if (cc0 == cc1) {
        int32_t const cell_index = (cc0[2] * num_cells_[1] + cc0[1]) * num_cells_[0] + cc0[0];

        Cell const& cell = cells_[cell_index];

        if (!cell.data) {
            c[0] = cell.value;
            c[1] = cell.value;
            c[2] = cell.value;
            c[3] = cell.value;
            c[4] = cell.value;
            c[5] = cell.value;
            c[6] = cell.value;
            c[7] = cell.value;

            return;
        }

        int3 const cs = cc0 << Log2_cell_dim;

        int32_t const d0 = (xyz[2] - cs[2]) << Log2_cell_dim;
        int32_t const d1 = (xyz1[2] - cs[2]) << Log2_cell_dim;

        int2 const csxy = cs.xy();

        {
            int2 const    cxy = xyz.xy() - csxy;
            int32_t const ci  = ((d0 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[0]              = cell.data[ci];
        }

        {
            int2 const    cxy = int2(xyz1[0], xyz[1]) - csxy;
            int32_t const ci  = ((d0 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[1]              = cell.data[ci];
        }

        {
            int2 const    cxy = int2(xyz[0], xyz1[1]) - csxy;
            int32_t const ci  = ((d0 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[2]              = cell.data[ci];
        }

        {
            int2 const    cxy = int2(xyz1[0], xyz1[1]) - csxy;
            int32_t const ci  = ((d0 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[3]              = cell.data[ci];
        }

        {
            int2 const    cxy = int2(xyz[0], xyz[1]) - csxy;
            int32_t const ci  = ((d1 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[4]              = cell.data[ci];
        }

        {
            int2 const    cxy = int2(xyz1[0], xyz[1]) - csxy;
            int32_t const ci  = ((d1 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[5]              = cell.data[ci];
        }

        {
            int2 const    cxy = int2(xyz[0], xyz1[1]) - csxy;
            int32_t const ci  = ((d1 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[6]              = cell.data[ci];
        }

        {
            int2 const    cxy = int2(xyz1[0], xyz1[1]) - csxy;
            int32_t const ci  = ((d1 + cxy[1]) << Log2_cell_dim) + cxy[0];
            c[7]              = cell.data[ci];
        }

        return;
    }

    c[0] = at(xyz[0], xyz[1], xyz[2]);
    c[1] = at(xyz1[0], xyz[1], xyz[2]);
    c[2] = at(xyz[0], xyz1[1], xyz[2]);
    c[3] = at(xyz1[0], xyz1[1], xyz[2]);
    c[4] = at(xyz[0], xyz[1], xyz1[2]);
    c[5] = at(xyz1[0], xyz[1], xyz1[2]);
    c[6] = at(xyz[0], xyz1[1], xyz1[2]);
    c[7] = at(xyz1[0], xyz1[1], xyz1[2]);
}

template <typename T>
int3 Typed_sparse_image<T>::coordinates_3(int64_t index) const {
    int64_t const w = int64_t(description_.dimensions_[0]);
    int64_t const h = int64_t(description_.dimensions_[1]);

    int64_t const area = w * h;
    int64_t const c2   = index / area;
    int64_t const t    = c2 * area;
    int64_t const c1   = (index - t) / w;

    return int3(index - (t + c1 * w), c1, c2);
}

template class Typed_image<uint8_t>;
template class Typed_image<byte2>;
template class Typed_image<byte3>;
template class Typed_image<byte4>;
template class Typed_image<ushort3>;
template class Typed_image<float>;
template class Typed_sparse_image<float>;
template class Typed_image<float2>;
template class Typed_image<packed_float3>;
template class Typed_image<float4>;

}  // namespace image
