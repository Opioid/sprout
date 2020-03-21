#include "typed_image.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"

#include <cstring>

namespace image {

Description::Description() : dimensions_(0), num_elements_(0) {}

Description::Description(int2 dimensions, int32_t num_elements)
    : dimensions_(dimensions, 1), num_elements_(num_elements) {}

Description::Description(int3 const& dimensions, int32_t num_elements)
    : dimensions_(dimensions), num_elements_(num_elements) {}

uint64_t Description::num_pixels() const {
    return uint64_t(dimensions_[0]) * uint64_t(dimensions_[1]) * uint64_t(dimensions_[2]) *
           uint64_t(num_elements_);
}

int2 Description::dimensions_2() const {
    return dimensions_.xy();
}

int3 const& Description::dimensions_3() const {
    return dimensions_;
}

int32_t Description::area() const {
    return dimensions_[0] * dimensions_[1];
}

int32_t Description::volume() const {
    return dimensions_[0] * dimensions_[1] * dimensions_[2];
}

int32_t Description::num_elements() const {
    return num_elements_;
}

template <typename T>
Typed_image<T>::Typed_image() : data_(nullptr) {}

template <typename T>
Typed_image<T>::Typed_image(Description const& description)
    : description_(description), data_(memory::allocate_aligned<T>(description.num_pixels())) {}

template <typename T>
Typed_image<T>::Typed_image(Typed_image&& other) noexcept
    : description_(other.description_), data_(other.data_) {
    other.data_ = nullptr;
}

template <typename T>
Typed_image<T>::~Typed_image() {
    memory::free_aligned(data_);
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

    memory::free_aligned(data_);

    description_.dimensions_   = description.dimensions_;
    description_.num_elements_ = description.num_elements_;

    data_ = memory::allocate_aligned<T>(description_.num_pixels());
}

template <typename T>
void Typed_image<T>::clear(T v) {
    for (int32_t i = 0, len = description_.volume(); i < len; ++i) {
        data_[i] = v;
    }
}

template <typename T>
T Typed_image<T>::load(int32_t index) const {
    return data_[index];
}

template <typename T>
T* Typed_image<T>::address(int32_t index) const {
    return data_ + index;
}

template <typename T>
void Typed_image<T>::store(int32_t index, T v) {
    data_[index] = v;
}

template <typename T>
T const& Typed_image<T>::at(int32_t index) const {
    return data_[index];
}

template <typename T>
T Typed_image<T>::load(int32_t x, int32_t y) const {
    int32_t const i = y * description_.dimensions_[0] + x;
    return data_[i];
}

template <typename T>
void Typed_image<T>::store(int32_t x, int32_t y, T v) {
    int32_t const i = y * description_.dimensions_[0] + x;
    data_[i]        = v;
}

template <typename T>
T Typed_image<T>::load_element(int32_t x, int32_t y, int32_t element) const {
    int32_t const i = (element * description_.dimensions_[1] + y) * description_.dimensions_[0] + x;
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at(int32_t x, int32_t y) const {
    int32_t const i = y * description_.dimensions_[0] + x;
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t element) const {
    int32_t const i = (element * description_.dimensions_[1] + y) * description_.dimensions_[0] + x;
    return data_[i];
}

template <typename T>
T Typed_image<T>::load(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(description_.dimensions_[1]) + int64_t(y)) *
                          int64_t(description_.dimensions_[0]) +
                      int64_t(x);
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(description_.dimensions_[1]) + int64_t(y)) *
                          int64_t(description_.dimensions_[0]) +
                      int64_t(x);
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t z, int32_t element) const {
    int3 const    d = description_.dimensions_;
    int32_t const i = ((element * d[2] + z) * d[1] + y) * d[0] + x;
    return data_[i];
}

template <typename T>
void Typed_image<T>::gather(int4 const& xy_xy1, T c[4]) const {
    int32_t const width = description_.dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = data_[y0 + xy_xy1[0]];
    c[1] = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    c[2] = data_[y1 + xy_xy1[0]];
    c[3] = data_[y1 + xy_xy1[2]];
}

template <typename T>
void Typed_image<T>::square_transpose() {
    int32_t const n = description_.dimensions_[0];
    for (int32_t y = 0, height = n - 2; y < height; ++y) {
        for (int32_t x = y + 1, width = n - 1; x < width; ++x) {
            int32_t const a = y * n + x;
            int32_t const b = x * n + y;
            std::swap(data_[a], data_[b]);
        }
    }
}

template <typename T>
T* Typed_image<T>::data() const {
    return data_;
}

template <typename T>
void Typed_image<T>::copy(Typed_image& destination) const {
    std::copy(&data_[0], &data_[description_.num_pixels() - 1], destination.data_);
}

template <typename T>
size_t Typed_image<T>::num_bytes() const {
    return sizeof(*this) +
           size_t(description_.dimensions_[0]) * size_t(description_.dimensions_[1]) *
               size_t(description_.dimensions_[2]) * size_t(description_.num_elements_) * sizeof(T);
}

// template <typename T>
// Typed_sparse_image<T>::Typed_sparse_image()  = default;

template <typename T>
Typed_sparse_image<T>::Typed_sparse_image(Description const& description)
    : description_(description) {
    int3 const d = description.dimensions_;

    num_cells_ = d >> Log2_cell_dim;

    num_cells_ += math::min(d - (num_cells_ << Log2_cell_dim), 1);

    int32_t const cell_len = num_cells_[0] * num_cells_[1] * num_cells_[2];

    cells_ = memory::allocate_aligned<Cell>(uint32_t(cell_len));

    for (int32_t i = 0; i < cell_len; ++i) {
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
        memory::free_aligned(cells_[i].data);
    }

    memory::free_aligned(cells_);
}

template <typename T>
Description const& Typed_sparse_image<T>::description() const {
    return description_;
}

template <typename T>
T Typed_sparse_image<T>::load(int64_t index) const {
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
void Typed_sparse_image<T>::store_sequentially(int64_t index, T v) {
    int3 const c  = coordinates_3(index);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    int32_t constexpr len = Cell_dim * Cell_dim * Cell_dim;

    Cell& cell = cells_[cell_index];

    if (!cell.data) {
        cell.data = memory::allocate_aligned<T>(len);

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
            memory::free_aligned(cell.data);

            cell.data  = nullptr;
            cell.value = value;
        }
    }
}

template <typename T>
T const& Typed_sparse_image<T>::at(int64_t index) const {
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
T Typed_sparse_image<T>::load(int32_t /*x*/, int32_t /*y*/) const {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    return data_[i];
    return T(0);
}

template <typename T>
void Typed_sparse_image<T>::store(int32_t /*x*/, int32_t /*y*/, T /*v*/) {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    data_[i]        = v;
}

template <typename T>
T Typed_sparse_image<T>::load_element(int32_t /*x*/, int32_t /*y*/, int32_t /*element*/) const {
    //    int32_t const i = (element * description_.dimensions[1] + y) * description_.dimensions[0]
    //    + x; return data_[i];

    return T(0);
}

template <typename T>
T const& Typed_sparse_image<T>::at(int32_t /*x*/, int32_t /*y*/) const {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    return data_[i];

    return empty_;
}

template <typename T>
T const& Typed_sparse_image<T>::at_element(int32_t /*x*/, int32_t /*y*/,
                                           int32_t /*element*/) const {
    //    int32_t const i = (element * description_.dimensions[1] + y) * description_.dimensions[0]
    //    + x; return data_[i];

    return empty_;
}

template <typename T>
T Typed_sparse_image<T>::load(int32_t x, int32_t y, int32_t z) const {
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
T const& Typed_sparse_image<T>::at(int32_t x, int32_t y, int32_t z) const {
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
T const& Typed_sparse_image<T>::at_element(int32_t /*x*/, int32_t /*y*/, int32_t /*z*/,
                                           int32_t /*element*/) const {
    //    int3 const    d = description_.dimensions;
    //    int32_t const i = ((element * d[2] + z) * d[1] + y) * d[0] + x;
    //    return data_[i];

    return empty_;
}

template <typename T>
void Typed_sparse_image<T>::gather(int4 const& /*xy_xy1*/, T c[4]) const {
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
size_t Typed_sparse_image<T>::num_bytes() const {
    uint32_t const cell_len = uint32_t(num_cells_[0] * num_cells_[1] * num_cells_[2]);

    size_t num_bytes = cell_len * sizeof(Cell);

    for (uint32_t i = 0; i < cell_len; ++i) {
        if (cells_[i].data) {
            num_bytes += Cell_dim * Cell_dim * Cell_dim * sizeof(T);
        }
    }

    return num_bytes;
}

template <typename T>
int3 Typed_sparse_image<T>::coordinates_3(int64_t index) const {
    int64_t const area = int64_t(description_.dimensions_[0]) *
                         int64_t(description_.dimensions_[1]);

    int64_t const c2 = index / area;

    int64_t const t = c2 * area;

    int64_t const c1 = (index - t) / int64_t(description_.dimensions_[0]);

    return int3(index - (t + c1 * int64_t(description_.dimensions_[0])), c1, c2);
}

template class Typed_image<uint8_t>;
template class Typed_image<byte2>;
template class Typed_image<byte3>;
template class Typed_image<byte4>;
template class Typed_image<short3>;
template class Typed_image<short4>;
template class Typed_image<float>;
template class Typed_sparse_image<float>;
template class Typed_image<float2>;
template class Typed_image<packed_float3>;
template class Typed_image<float4>;

}  // namespace image
