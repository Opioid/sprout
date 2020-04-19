#include "file.hpp"

#include <cstring>
#include <istream>

namespace file {

Type query_type(std::istream& stream) {
    char header[4];
    stream.read(header, sizeof(header));

    stream.seekg(0);

    if (!strncmp("\x76\x2F\x31\x01", header, 4)) {
        return Type::EXR;
    }

    if (!strncmp("\037\213", header, 2)) {
        return Type::GZIP;
    }

    if (!strncmp("\211PNG", header, 4)) {
        return Type::PNG;
    }

    if (!strncmp("#?", header, 2)) {
        return Type::RGBE;
    }

    if (!strncmp("SUB\000", header, 4)) {
        return Type::SUB;
    }

    if (!strncmp("\x28\xB5\x2F\xFD", header, 4)) {
        return Type::ZSTD;
    }

    return Type::Undefined;
}

}  // namespace file
