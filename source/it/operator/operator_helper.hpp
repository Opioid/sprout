#ifndef SU_IT_OPERATOR_HELPER_HPP
#define SU_IT_OPERATOR_HELPER_HPP

#include "core/image/typed_image_fwd.hpp"

#include <string>

namespace thread {
class Pool;
};

namespace op {

void write(image::Float4 const& image, std::string const& name, bool alpha, thread::Pool& threads);

}

#endif
