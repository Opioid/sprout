#include "file_system.hpp"
#include "base/string/string.hpp"
#include "file.hpp"
#include "logging/logging.hpp"
#include "read_stream.inl"

namespace file {

System::Stream_ptr::Stream_ptr(System& system, Type type) : system_(system), type_(type) {}

System::Stream_ptr::Stream_ptr(Stream_ptr&& other) noexcept
    : system_(other.system_), type_(other.type_) {
    other.type_ = Type::Invalid;
}

System::Stream_ptr::~Stream_ptr() {
    close();
}

System::Stream_ptr::operator bool() const {
    return Type::Invalid != type_;
}

bool System::Stream_ptr::operator!() const {
    return Type::Invalid == type_;
}

std::istream& System::Stream_ptr::operator*() const {
    return system_.stream(*this);
}

void System::Stream_ptr::close() {
    if (Type::Invalid != type_) {
        system_.close(*this);
    }
}

System::System() : read_buffer_size_(0), buffer_size_(0), read_buffer_(nullptr), buffer_(nullptr) {}

System::~System() {
    delete[] read_buffer_;
    delete[] buffer_;
}

System::Stream_ptr System::read_stream(std::string_view name) {
    std::string resolved_name;
    return read_stream(name, resolved_name);
}

System::Stream_ptr System::read_stream(std::string_view name, std::string& resolved_name) {
    auto& stream = open_read_stream(name, resolved_name);
    if (!stream) {
        logging::push_error("Stream %S could not be opened.", std::string(name));
        return Stream_ptr(*this, Stream_ptr::Type::Invalid);
    }

    const Type type = query_type(stream);

    if (Type::GZIP == type) {
        allocate_buffers(gzip::Filebuffer::read_buffer_size(),
                         gzip::Filebuffer::write_buffer_size());

        gzip_stream_.clear();
        gzip_stream_.open(&stream, read_buffer_size_, read_buffer_, buffer_size_, buffer_);

        return Stream_ptr(*this, Stream_ptr::Type::GZIP);
    }

    if (Type::ZSTD == type) {
#ifdef SU_ZSTD
        allocate_buffers(zstd::Filebuffer::read_buffer_size(),
                         zstd::Filebuffer::write_buffer_size());

        zstd_stream_.clear();
        zstd_stream_.open(&stream, read_buffer_size_, read_buffer_, buffer_size_, buffer_);

        return Stream_ptr(*this, Stream_ptr::Type::ZSTD);
#else
        logging::push_error("ZSTD compression is not supported.");
        return Stream_ptr(*this, Stream_ptr::Type::Invalid);
#endif
    }

    return Stream_ptr(*this, Stream_ptr::Type::Uncompressed);
}

System::Stream_ptr System::string_stream(std::string const& string) {
    str_stream_.clear();
    str_stream_.str(string);

    return Stream_ptr(*this, Stream_ptr::Type::String);
}

std::istream& System::stream(Stream_ptr const& ptr) {
    if (Stream_ptr::Type::GZIP == ptr.type_) {
        return gzip_stream_;
    }

#ifdef SU_ZSTD
    if (Stream_ptr::Type::ZSTD == ptr.type_) {
        return zstd_stream_;
    }
#endif

    if (Stream_ptr::Type::String == ptr.type_) {
        return str_stream_;
    }

    return stream_;
}

void System::close(Stream_ptr& stream) {
    if (Stream_ptr::Type::Invalid == stream.type_) {
        return;
    }

    if (Stream_ptr::Type::Uncompressed == stream.type_) {
        stream_.close();
    } else if (Stream_ptr::Type::GZIP == stream.type_) {
        gzip_stream_.close();
        stream_.close();
    } else if (Stream_ptr::Type::ZSTD == stream.type_) {
#ifdef SU_ZSTD
        zstd_stream_.close();
        stream_.close();
#endif
    } else if (Stream_ptr::Type::String == stream.type_) {
        str_stream_.str(std::string());
    }

    stream.type_ = Stream_ptr::Type::Invalid;
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

void System::set_frame(uint32_t frame) {
    frame_string_ = string::to_string(frame);
}

std::istream& System::open_read_stream(std::string_view name, std::string& resolved_name) {
    std::string modified_name = std::string(name);

    if (size_t const pos = modified_name.find("{FRAME}"); std::string::npos != pos) {
        modified_name.replace(pos, 7, frame_string_);
    }

    for (auto const& f : mount_folders_) {
        // Ignore empty folders, because this is handled explicitely
        if (f.empty()) {
            continue;
        }

        stream_.close();
        stream_.clear();

        resolved_name = f + modified_name;
        stream_.open(resolved_name, std::ios::binary);
        if (stream_) {
            return stream_;
        }
    }

    stream_.close();
    stream_.clear();

    stream_.open(modified_name, std::ios::binary);
    if (stream_) {
        resolved_name = modified_name;
        return stream_;
    }

    return stream_;
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
