#include "file_system.hpp"
#include "base/memory/unique.inl"
#include "file.hpp"
#include "gzip_read_stream.hpp"
#include "logging/logging.hpp"
#include "read_stream.inl"
#include "zstd_read_stream.hpp"

#include <fstream>
#include <string_view>

namespace file {

System::System() : read_buffer_size_(0), buffer_size_(0), read_buffer_(nullptr), buffer_(nullptr) {}

System::~System() {
    delete[] read_buffer_;
    delete[] buffer_;
}

Stream_ptr System::read_stream(std::string_view name) {
    std::string resolved_name;
    return read_stream(name, resolved_name);
}

Stream_ptr System::read_stream(std::string_view name, std::string& resolved_name) {
    auto stream = open_read_stream(name, resolved_name);
    if (!stream) {
        logging::push_error("Stream %S could not be opened.", std::string(name));
        return Stream_ptr();
    }

    const Type type = query_type(*stream);

    if (Type::GZIP == type) {
        allocate_buffers(gzip::Filebuffer::read_buffer_size(),
                         gzip::Filebuffer::write_buffer_size());

        return Stream_ptr(new Read_stream<gzip::Filebuffer>(stream, read_buffer_size_, read_buffer_,
                                                            buffer_size_, buffer_));
    }

    if (Type::ZSTD == type) {
        allocate_buffers(zstd::Filebuffer::read_buffer_size(),
                         zstd::Filebuffer::write_buffer_size());

        return Stream_ptr(new Read_stream<zstd::Filebuffer>(stream, read_buffer_size_, read_buffer_,
                                                            buffer_size_, buffer_));
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

std::istream* System::open_read_stream(std::string_view name, std::string& resolved_name) {
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

void System::allocate_buffers(uint32_t read_size, uint32_t size) {
    if (read_size > read_buffer_size_) {
        delete[] read_buffer_;

        read_buffer_size_ = read_size;
        read_buffer_      = new char[read_size];
    }

    if (size > buffer_size_) {
        delete[] buffer_;

        buffer_size_ = size;
        buffer_      = new char[size];
    }
}

}  // namespace file

template class memory::Unique_ptr<std::istream>;
