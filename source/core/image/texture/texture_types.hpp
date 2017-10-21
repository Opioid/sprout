#pragma once

#include "texture_adapter.hpp"
#include <memory>

namespace image::texture { class Texture; }

using Texture_ptr = std::shared_ptr<image::texture::Texture>;

using Texture_adapter = image::texture::Adapter;
