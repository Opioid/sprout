#include "file_system.hpp"
#include "base/memory/unique.inl"
#include "file.hpp"
#include "gzip/gzip_read_stream.hpp"
#include "zstd/zstd_read_stream.hpp"
#include "logging/logging.hpp"

#include <fstream>
#include <string_view>

namespace file {

Stream_ptr System::read_stream(std::string_view name) const {
    std::string resolved_name;
    return read_stream(name, resolved_name);
}

Stream_ptr System::read_stream(std::string_view name, std::string& resolved_name) const {
    auto stream = open_read_stream(name, resolved_name);
    if (!stream) {
        logging::push_error("Stream %S could not be opened.", std::string(name));
        return Stream_ptr();
    }

    const Type type = query_type(*stream);

    if (Type::GZIP == type) {
        return Stream_ptr(new gzip::Read_stream(stream));
    }

    if (Type::ZSTD == type) {
        return Stream_ptr(new zstd::Read_stream(stream));
    }

    return Stream_ptr(stream);
}

void System::push_mount(std::string_view folder) {
    // We also have to push empty folders, otherwise popping gets complicated

    std::string buffer(folder);

    if (!folder.empty() && folder.back() != '/') {
        buffer.append("/");
    }

    mount_folders_.push_back(buffer);
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

template class memory::Unique_ptr<std::istream>;
