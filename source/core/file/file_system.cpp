#include "file_system.hpp"
#include <fstream>
#include <sstream>
#include <string_view>
#include "file.hpp"
#include "gzip/gzip_read_stream.hpp"

namespace file {

std::unique_ptr<std::istream> System::read_stream(std::string_view name) const {
    std::string resolved_name;
    return read_stream(name, resolved_name);
}

std::unique_ptr<std::istream> System::read_stream(std::string_view name,
                                                  std::string&     resolved_name) const {
    auto stream = open_read_stream(name, resolved_name);
    if (!stream) {
        throw std::runtime_error("Stream \"" + std::string(name) + "\" could not be opened");
    }

    const Type type = query_type(*stream);

    if (Type::GZIP == type) {
        return std::make_unique<gzip::Read_stream>(stream);
    }

    return std::unique_ptr<std::istream>(stream);
}

void System::push_mount(std::string_view folder) {
    // We also have to push empty folders, otherwise popping gets complicated

    std::stringstream stream;
    stream << folder;

    if (!folder.empty() && folder.back() != '/') {
        stream << "/";
    }

    mount_folders_.push_back(stream.str());
}

void System::pop_mount() {
    mount_folders_.pop_back();
}

std::istream* System::open_read_stream(std::string_view name, std::string& resolved_name) const {
    // TODO: Use something like std::filesytem::exists() when it is available

    for (auto const& f : mount_folders_) {
        // Ignore empty folders, because this is handled explicitely
        if (f.empty()) {
            continue;
        }

        resolved_name = f + std::string(name);

        std::istream* stream = new std::ifstream(resolved_name, std::ios::binary);
        if (*stream) {
            return stream;
        }

        delete stream;
    }

    std::istream* stream = new std::ifstream(name.data(), std::ios::binary);
    if (*stream) {
        resolved_name = name;
        return stream;
    }

    delete stream;
    return nullptr;
}

}  // namespace file
