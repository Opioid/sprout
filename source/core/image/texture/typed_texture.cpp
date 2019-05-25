#include "typed_texture.hpp"
#include "typed_texture.hpp"
#include "base/math/vector3.inl"
#include "image/typed_image.hpp"

namespace image::texture {


template <typename T>
Typed_texture<T>::Typed_texture(T const& image) : image_(image) {}

template <typename T>
Image const& Typed_texture<T>::image() const noexcept {
return image_;
}

template <typename T>
int32_t Typed_texture<T>::num_channels() const noexcept {
return image_.description().num_channels();
}

template <typename T>
int32_t Typed_texture<T>::num_elements() const noexcept {
return image_.description().num_elements;
}

template <typename T>
int2        Typed_texture<T>::dimensions_2() const noexcept {
 return image_.description().dimensions.xy();
}

template <typename T>
int3 const& Typed_texture<T>::dimensions_3() const noexcept {
 return image_.description().dimensions;
}

template <typename T>
size_t Typed_texture<T>::image_num_bytes() const noexcept {
 return image_.num_bytes();
}

template class Typed_texture<Byte1>;
template class Typed_texture<Byte2>;
template class Typed_texture<Byte3>;
template class Typed_texture<Byte4>;
template class Typed_texture<Float1>;
template class Typed_texture<Float1_sparse>;
template class Typed_texture<Float2>;
template class Typed_texture<Float3>;


}
