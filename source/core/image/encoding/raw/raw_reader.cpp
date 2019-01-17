#include "raw_reader.hpp"
#include <istream>
#include <string>
#include "base/math/vector4.inl"
#include "image/encoding/sub/sub_image_writer.hpp"
#include "image/typed_image.hpp"

namespace image::encoding::raw {

Image* Reader::read(std::istream& stream) const {
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

    int3 const dimensions(1987, 1351, 2449);

    Image::Description description(Image::Type::Float1, dimensions);

    auto volume = new Float1(description);

    uint64_t const num_voxels = static_cast<uint64_t>(dimensions[0]) *
                                static_cast<uint64_t>(dimensions[1]) *
                                static_cast<uint64_t>(dimensions[2]);

    stream.read(reinterpret_cast<char*>(volume->data()), num_voxels * sizeof(float));

    //    sub::Writer::write("disney_cloud.sub", *volume);

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
