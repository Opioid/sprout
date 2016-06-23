#pragma once

#include <memory>

namespace image { namespace texture { class Texture_2D; }}

using Texture_2D_ptr = std::shared_ptr<image::texture::Texture_2D>;
