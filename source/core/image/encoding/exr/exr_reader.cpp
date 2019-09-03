#include "exr_reader.hpp"
#include <cstdint>
#include <fstream>
#include "base/math/vector2.inl"
#include "exr.inl"
#include "image/image.hpp"
#include "logging/logging.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace image::encoding::exr {

void channel_list(std::istream& stream) noexcept;
void compression(std::istream& stream) noexcept;

Image* Reader::read(std::istream& stream) noexcept {
    uint8_t header[Signature_size];

    // Check signature
    stream.read(reinterpret_cast<char*>(header), Signature_size);

    if (memcmp(reinterpret_cast<const void*>(Signature), reinterpret_cast<void*>(header),
               Signature_size)) {
        logging::push_error("Bad EXR signature");
        return nullptr;
    }

    // Check version
    stream.read(reinterpret_cast<char*>(header), Signature_size);

    std::cout << uint32_t(header[0]) << " " << uint32_t(header[1]) << " " << uint32_t(header[2])
              << " " << uint32_t(header[3]) << std::endl;

    std::string attribute;

    for (;;) {
        std::getline(stream, attribute, '\0');

        if (attribute.empty()) {
            break;
        }

        std::cout << attribute << ": ";

        if ("channels" == attribute) {
        }

        std::getline(stream, attribute, '\0');

        std::cout << attribute << ": ";

        int32_t attribute_size;
        stream.read(reinterpret_cast<char*>(&attribute_size), sizeof(int32_t));

        std::cout << attribute_size << ": ";

        if ("box2i" == attribute) {
            int4 box;
            stream.read(reinterpret_cast<char*>(&box), sizeof(int4));
            std::cout << box;
        } else if ("chlist" == attribute) {
            channel_list(stream);
        } else if ("compression" == attribute) {
            compression(stream);
        } else {
            stream.seekg(attribute_size, std::ios_base::cur);
        }

        std::cout << std::endl;
    }

    int32_t v;
    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    stream.read(reinterpret_cast<char*>(&v), sizeof(int32_t));
    std::cout << v << std::endl;

    return nullptr;
}

void compression(std::istream& stream) noexcept {
    uint8_t type;
    stream.read(reinterpret_cast<char*>(&type), sizeof(uint8_t));

    if (0 == type) {
        std::cout << "NO_COMPRESSION";
    } else if (1 == type) {
        std::cout << "RLE_COMPRESSION";
    } else if (2 == type) {
        std::cout << "ZIPS_COMPRESSION";
    } else if (3 == type) {
        std::cout << "ZIP_COMPRESSION";
    } else if (4 == type) {
        std::cout << "PIZ_COMPRESSION";
    } else if (5 == type) {
        std::cout << "PXR24_COMPRESSION";
    } else if (6 == type) {
        std::cout << "B44_COMPRESSION";
    } else if (7 == type) {
        std::cout << "B44A_COMPRESSION";
    }
}

void channel_list(std::istream& stream) noexcept {
    std::cout << "[ ";

    for (int32_t i = 0;; ++i) {
        std::string name;
        std::getline(stream, name, '\0');

        if (name.empty()) {
            break;
        } else if (i > 0) {
            std::cout << ", ";
        }

        int32_t type;
        stream.read(reinterpret_cast<char*>(&type), sizeof(int32_t));

        // pLinear
        stream.seekg(1, std::ios_base::cur);

        // reserved
        stream.seekg(3, std::ios_base::cur);

        // xSampling ySampling
        stream.seekg(4 + 4, std::ios_base::cur);

        std::cout << name << " ";

        if (0 == type) {
            std::cout << "uint";
        } else if (1 == type) {
            std::cout << "half";
        } else if (2 == type) {
            std::cout << "float";
        }
    }

    std::cout << " ]";
}

}  // namespace image::encoding::exr
