#include "raw_reader.hpp"
#include <istream>
#include <string>
#include "base/math/vector4.inl"
#include "image/encoding/sub/sub_image_writer.hpp"
#include "image/typed_image.hpp"

namespace image::encoding::raw {

Image* Reader::read(std::istream& stream) noexcept {
    /*
    int3 const dimensions(512, 512, 32);

    Image::Description description(Image::Type::Byte1, dimensions);

    auto volume = std::make_shared<Byte1>(description);

    int32_t const num_bytes = dimensions[0] * dimensions[1] * dimensions[2];
    stream.read(reinterpret_cast<char*>(volume->data()), num_bytes);

    sub::Writer::write("clouds.sub", *volume);

    return volume;
    */

    // eighth
    // int3 const dimensions(250, 170, 307);

    //  int3 const dimensions(216, 379, 215);

    int3 const dimensions(69, 171, 67);

    Description description(dimensions);

    auto volume = new Image(Float1(description));

    uint64_t const num_voxels = uint64_t(dimensions[0]) * uint64_t(dimensions[1]) *
                                uint64_t(dimensions[2]);

    stream.read(reinterpret_cast<char*>(volume->float1().data()), num_voxels * sizeof(float));

    //   sub::Writer::write("rocket_111.sub", *volume);

    return volume;

    //            int3 const dimensions(2, 2, 2);

    //            Image::Description description(Image::Type::Float1, dimensions);

    //            auto volume = std::make_shared<Float1>(description);

    //        volume->at(0, 0, 0) = 0.1f; volume->at(1, 0, 0) = 0.1f;
    //        volume->at(0, 1, 0) = 0.9f; volume->at(1, 1, 0) = 0.9f;

    //        volume->at(0, 0, 1) = 0.1f; volume->at(1, 0, 1) = 0.1f;
    //        volume->at(0, 1, 1) = 0.9f; volume->at(1, 1, 1) = 0.9f;

    //            volume->at(0, 0, 0) = 0.1f;   volume->at(1, 0, 0) = 0.2f;  volume->at(2, 0, 0) =
    //            0.3f; volume->at(0, 1, 0) = 0.25f;  volume->at(1, 1, 0) = 0.f;   volume->at(2, 1,
    //            0) = 0.75f; volume->at(0, 2, 0) = 0.25f;  volume->at(1, 2, 0) = 0.5f;
    //            volume->at(2, 2, 0) = 0.75f;

    //            volume->at(0, 0, 1) = 0.4f;  volume->at(1, 0, 1) = 0.5f;   volume->at(2, 0, 1) =
    //            0.6f; volume->at(0, 1, 1) = 0.5f;  volume->at(1, 1, 1) = 0.25f;  volume->at(2, 1,
    //            1) = 0.f; volume->at(0, 2, 1) = 0.f;   volume->at(1, 2, 1) = 0.25f;  volume->at(2,
    //            2, 1) = 0.5f;

    //            volume->at(0, 0, 2) = 0.7f;   volume->at(1, 0, 2) = 0.8f;  volume->at(2, 0, 2) =
    //            0.9f; volume->at(0, 1, 2) = 0.75f;  volume->at(1, 1, 2) = 0.5f;  volume->at(2, 1,
    //            2) = 0.25f; volume->at(0, 2, 2) = 0.75f;  volume->at(1, 2, 2) = 0.f; volume->at(2,
    //            2, 2) = 0.25f;

    // return volume;
}

}  // namespace image::encoding::raw
