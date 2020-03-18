#ifndef SU_IT_OPERATOR_HELPER_HPP
#define SU_IT_OPERATOR_HELPER_HPP

#include "core/image/typed_image_fwd.hpp"

#include <string>

namespace thread {
class Pool;
};

namespace op {

bool write(image::Float4 const& image, std::string const& name, bool alpha, thread::Pool& threads);

enum class Transcode {
    RGBE = 0,
    RGBD = 1
};

bool write(image::Float4 const& image, std::string const& name, Transcode transcode, thread::Pool& threads);

}

#endif
