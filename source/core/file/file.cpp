#include "file.hpp"
#include <cstring>
#include <istream>

namespace file {

Type query_type(std::istream& stream) {
    char header[4];
    stream.read(header, sizeof(header));

    stream.seekg(0);

    if (!strncmp("\x76\x2f\x31\x01", header, 4)) {
        return Type::EXR;
    } else if (!strncmp("\037\213", header, 2)) {
        return Type::GZIP;
    } else if (!strncmp("\211PNG", header, 4)) {
        return Type::PNG;
    } else if (!strncmp("#?", header, 2)) {
        return Type::RGBE;
    } else if (!strncmp("SUB\000", header, 4)) {
        return Type::SUB;
    } else {
        return Type::Undefined;
    }
}

}  // namespace file
