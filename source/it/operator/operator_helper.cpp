#include "operator_helper.hpp"
#include "base/string/string.hpp"
#include "base/thread/thread_pool.hpp"
#include "core/image/typed_image.hpp"
#include "core/image/encoding/exr/exr_writer.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/encoding/rgbe/rgbe_as_png_writer.hpp"
#include "core/image/encoding/rgbe/rgbe_writer.hpp"

#include <fstream>

namespace op {

using namespace image;

bool write(Float4 const& image, std::string const& name, bool alpha, thread::Pool& threads) {
    std::string_view const s = string::suffix(name);

    std::ofstream stream(name, std::ios::binary);

    if ("exr" == s) {
        encoding::exr::Writer writer(alpha);

        std::string_view ps = string::presuffix(name);

        if ("rgbe" == ps) {
            image::Float4 temp(image.description());
            encoding::rgbe::Rgbe_as_png::transcode(image, temp, threads);
            return writer.write(stream, temp, threads);
        }

        if ("rgbd" == ps) {
            image::Float4 temp(image.description());
            encoding::rgbe::Rgbd_as_png::transcode(image, temp, threads);
            return writer.write(stream, temp, threads);
        }

        return writer.write(stream, image, threads);
    }

    if ("hdr" == s) {
        encoding::rgbe::Writer writer;

        std::string_view ps = string::presuffix(name);

        if ("rgbe" == ps) {
            image::Float4 temp(image.description());
            encoding::rgbe::Rgbe_as_png::transcode(image, temp, threads);
            return writer.write(stream, temp, threads);
        }

        if ("rgbd" == ps) {
            image::Float4 temp(image.description());
            encoding::rgbe::Rgbd_as_png::transcode(image, temp, threads);
            return writer.write(stream, temp, threads);
        }

        return writer.write(stream, image, threads);
    }

    if ("png" == s) {
        std::string_view ps = string::presuffix(name);

        if ("rgbe" == ps) {
            encoding::rgbe::Rgbe_as_png writer;
            return writer.write(stream, image, threads);
        }

        if ("rgbd" == ps) {
            encoding::rgbe::Rgbd_as_png writer;
            return writer.write(stream, image, threads);
        }
    }

    encoding::png::Writer writer(false, alpha, false);
    return writer.write(stream, image, threads);
}

}  // namespace op
