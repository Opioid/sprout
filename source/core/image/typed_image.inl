#ifndef SU_CORE_IMAGE_TYPED_IMAGE_INL
#define SU_CORE_IMAGE_TYPED_IMAGE_INL

#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "typed_image.hpp"
#include "typed_image_fwd.hpp"

namespace image {

template <typename T>
Typed_image<T>::Typed_image(const Image::Description& description) noexcept
    : Image(description), data_(memory::allocate_aligned<T>(description.num_pixels())) {}

template <typename T>
Typed_image<T>::~Typed_image() noexcept {
    memory::free_aligned(data_);
}

template <typename T>
Typed_image<T> Typed_image<T>::clone() const noexcept {
    return Typed_image<T>(description_);
}

template <typename T>
void Typed_image<T>::resize(const Image::Description& description) noexcept {
    memory::free_aligned(data_);

    Image::resize(description);

    data_ = memory::allocate_aligned<T>(description.num_pixels());
}

template <typename T>
void Typed_image<T>::clear(T v) noexcept {
    for (int32_t i = 0, len = volume(); i < len; ++i) {
        data_[i] = v;
    }
}

template <typename T>
T Typed_image<T>::load(int32_t index) const noexcept {
    return data_[index];
}

template <typename T>
T* Typed_image<T>::address(int32_t index) const noexcept {
    return data_ + index;
}

template <typename T>
void Typed_image<T>::store(int32_t index, T v) noexcept {
    data_[index] = v;
}

template <typename T>
T const& Typed_image<T>::at(int32_t index) const noexcept {
    return data_[index];
}

template <typename T>
T Typed_image<T>::load(int32_t x, int32_t y) const noexcept {
    int32_t const i = y * description_.dimensions[0] + x;
    return data_[i];
}

template <typename T>
void Typed_image<T>::store(int32_t x, int32_t y, T v) noexcept {
    int32_t const i = y * description_.dimensions[0] + x;
    data_[i]        = v;
}

template <typename T>
T Typed_image<T>::load_element(int32_t x, int32_t y, int32_t element) const noexcept {
    int32_t const i = (element * description_.dimensions[1] + y) * description_.dimensions[0] + x;
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at(int32_t x, int32_t y) const noexcept {
    int32_t const i = y * description_.dimensions[0] + x;
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t element) const noexcept {
    int32_t const i = (element * description_.dimensions[1] + y) * description_.dimensions[0] + x;
    return data_[i];
}

template <typename T>
T Typed_image<T>::load(int32_t x, int32_t y, int32_t z) const noexcept {
    int32_t const i = (z * description_.dimensions[1] + y) * description_.dimensions[0] + x;
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at(int32_t x, int32_t y, int32_t z) const noexcept {
    int32_t const i = (z * description_.dimensions[1] + y) * description_.dimensions[0] + x;
    return data_[i];
}

template <typename T>
T const& Typed_image<T>::at_element(int32_t x, int32_t y, int32_t z, int32_t element) const
    noexcept {
    int3 const    d = description_.dimensions;
    int32_t const i = ((element * d[2] + z) * d[1] + y) * d[0] + x;
    return data_[i];
}

template <typename T>
void Typed_image<T>::gather(int4 const& xy_xy1, T c[4]) const noexcept {
    int32_t const width = description_.dimensions[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = data_[y0 + xy_xy1[0]];
    c[1] = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    c[2] = data_[y1 + xy_xy1[0]];
    c[3] = data_[y1 + xy_xy1[2]];
}

template <typename T>
void Typed_image<T>::square_transpose() noexcept {
    int32_t const n = description_.dimensions[0];
    for (int32_t y = 0, height = n - 2; y < height; ++y) {
        for (int32_t x = y + 1, width = n - 1; x < width; ++x) {
            int32_t const a = y * n + x;
            int32_t const b = x * n + y;
            std::swap(data_[a], data_[b]);
        }
    }
}

template <typename T>
T* Typed_image<T>::data() const noexcept {
    return data_;
}

template <typename T>
size_t Typed_image<T>::num_bytes() const noexcept {
    return sizeof(*this) + description_.dimensions[0] * description_.dimensions[1] *
                               description_.dimensions[2] * description_.num_elements * sizeof(T);
}

template <typename T>
Typed_sparse_image<T>::Typed_sparse_image(const Image::Description& description) noexcept
    : Image(description) {
    int3 const d = description.dimensions;

    num_cells_ = d >> Log2_cell_dim;

    num_cells_ += math::min(d - (num_cells_ << Log2_cell_dim), 1);

    uint32_t cell_len = static_cast<uint32_t>(num_cells_[0] * num_cells_[1] * num_cells_[2]);

    cells_ = memory::allocate_aligned<T*>(cell_len);

    for (uint32_t i = 0; i < cell_len; ++i) {
        cells_[i] = nullptr;
    }
}

template <typename T>
Typed_sparse_image<T>::~Typed_sparse_image() noexcept {
    uint32_t cell_len = static_cast<uint32_t>(num_cells_[0] * num_cells_[1] * num_cells_[2]);

    for (uint32_t i = 0; i < cell_len; ++i) {
        memory::free_aligned(cells_[i]);
    }
}

template <typename T>
T Typed_sparse_image<T>::load(int64_t index) const noexcept {
    int3 const c  = coordinates_3(index);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    if (!cells_[cell_index]) {
        return T(0);
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    return cells_[cell_index][ci];
}

template <typename T>
void Typed_sparse_image<T>::store(int64_t index, T v) noexcept {
    int3 const c  = coordinates_3(index);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    if (!cells_[cell_index]) {
        uint64_t const len = static_cast<uint64_t>(Cell_dim * Cell_dim * Cell_dim);
        cells_[cell_index] = memory::allocate_aligned<T>(len);

        std::memset(cells_[cell_index], 0, len * sizeof(T));
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    cells_[cell_index][ci] = v;
}

template <typename T>
T const& Typed_sparse_image<T>::at(int64_t index) const noexcept {
    int3 const c  = coordinates_3(index);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    if (!cells_[cell_index]) {
        return T(0);
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    return cells_[cell_index][ci];
}

template <typename T>
T Typed_sparse_image<T>::load(int32_t /*x*/, int32_t /*y*/) const noexcept {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    return data_[i];
    return T(0);
}

template <typename T>
void Typed_sparse_image<T>::store(int32_t /*x*/, int32_t /*y*/, T /*v*/) noexcept {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    data_[i]        = v;
}

template <typename T>
T Typed_sparse_image<T>::load_element(int32_t /*x*/, int32_t /*y*/, int32_t /*element*/) const
    noexcept {
    //    int32_t const i = (element * description_.dimensions[1] + y) * description_.dimensions[0]
    //    + x; return data_[i];

    return T(0);
}

template <typename T>
T const& Typed_sparse_image<T>::at(int32_t /*x*/, int32_t /*y*/) const noexcept {
    //    int32_t const i = y * description_.dimensions[0] + x;
    //    return data_[i];

    return empty_;
}

template <typename T>
T const& Typed_sparse_image<T>::at_element(int32_t /*x*/, int32_t /*y*/, int32_t /*element*/) const
    noexcept {
    //    int32_t const i = (element * description_.dimensions[1] + y) * description_.dimensions[0]
    //    + x; return data_[i];

    return empty_;
}

template <typename T>
T Typed_sparse_image<T>::load(int32_t x, int32_t y, int32_t z) const noexcept {
    int3 const c(x, y, z);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    if (!cells_[cell_index]) {
        return T(0);
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    return cells_[cell_index][ci];
}

template <typename T>
T const& Typed_sparse_image<T>::at(int32_t x, int32_t y, int32_t z) const noexcept {
    int3 const c(x, y, z);
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    if (!cells_[cell_index]) {
        return empty_;
    }

    int3 const cs = cc << Log2_cell_dim;

    int3 const cxyz = c - cs;

    int32_t ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

    return cells_[cell_index][ci];
}

template <typename T>
T const& Typed_sparse_image<T>::at_element(int32_t /*x*/, int32_t /*y*/, int32_t /*z*/,
                                           int32_t /*element*/) const noexcept {
    //    int3 const    d = description_.dimensions;
    //    int32_t const i = ((element * d[2] + z) * d[1] + y) * d[0] + x;
    //    return data_[i];

    return empty_;
}

template <typename T>
void Typed_sparse_image<T>::gather(int4 const& /*xy_xy1*/, T c[4]) const noexcept {
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
size_t Typed_sparse_image<T>::num_bytes() const noexcept {
    uint32_t cell_len = static_cast<uint32_t>(num_cells_[0] * num_cells_[1] * num_cells_[2]);

    size_t num_bytes = cell_len * sizeof(T*);

    for (uint32_t i = 0; i < cell_len; ++i) {
        if (cells_[i]) {
            num_bytes += Cell_dim * Cell_dim * Cell_dim * sizeof(T);
        }
    }

    return num_bytes;
}

}  // namespace image

#endif
