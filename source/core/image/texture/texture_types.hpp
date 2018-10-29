#ifndef SU_CORE_IMAGE_TEXTURE_TYPES_HPP
#define SU_CORE_IMAGE_TEXTURE_TYPES_HPP

#include <memory>

namespace image::texture {

class Texture;
class Adapter;

}  // namespace image::texture

using Texture_ptr = std::shared_ptr<image::texture::Texture>;

using Texture_adapter = image::texture::Adapter;

#endif
