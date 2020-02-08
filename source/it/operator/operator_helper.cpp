#include "operator_helper.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/encoding/exr/exr_writer.hpp"
#include "core/image/encoding/png/png_writer.hpp"

#include <fstream>

namespace op {

using namespace image;

void write(Float4 const& image, std::string const& name, bool alpha, thread::Pool& threads) {
    bool const exr = "exr" == string::suffix(name);

    std::ofstream stream(name, std::ios::binary);

    if (exr) {
        encoding::exr::Writer writer(alpha);
        writer.write(stream, image, threads);
    } else {
        if (alpha) {
            encoding::png::Writer_alpha writer(false, false);
            writer.write(stream, image, threads);
        } else {
            encoding::png::Writer writer(false);
            writer.write(stream, image, threads);
        }
    }
}

}  // namespace op
