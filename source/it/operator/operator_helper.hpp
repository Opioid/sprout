#ifndef SU_IT_OPERATOR_HELPER_HPP
#define SU_IT_OPERATOR_HELPER_HPP

#include "core/image/typed_image_fwd.hpp"

#include <string>

namespace thread {
class Pool;
};

using Threads = thread::Pool;

namespace op {

bool write(image::Float4 const& image, std::string const& name, bool alpha, Threads& threads);

}

#endif
