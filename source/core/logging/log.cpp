#include "log.hpp"
#include "base/string/string.hpp"

namespace logging {

static std::string resolve(std::string const& text, float a) noexcept {
    auto const i = text.find("%f");

    return text.substr(0, i) + string::to_string(a) + text.substr(i + 2);
}

static std::string resolve(std::string const& text, std::string const& a) noexcept {
    auto const i = text.find("%S");

    return text.substr(0, i) + "\"" + a + "\"" + text.substr(i + 2);
}

Log::~Log() {}

void Log::post(Type type, std::string const& text) noexcept {
    if (0 == current_entry_) {
        internal_post(type, text);
    } else {
        std::string concat = text;

        for (int32_t i = current_entry_ - 1; i >= 0; --i) {
            Entry const& entry = entries_[i];
            if (entry.type == type) {
                concat += entry.text;
            }
        }

        current_entry_ = 0;

        internal_post(type, concat);
    }
}

void Log::post(Type type, std::string const& text, float a) noexcept {
    post(type, resolve(text, a));
}

void Log::post(Type type, std::string const& text, std::string const& a) noexcept {
    post(type, resolve(text, a));
}

void Log::push(Type type, std::string const& text) noexcept {
    if (Num_entries == current_entry_) {
        post(type);
    }

    Entry& entry = entries_[current_entry_];
    entry.type   = type;
    entry.text   = text;

    ++current_entry_;
}

void Log::push(Type type, std::string const& text, std::string const& a) noexcept {
    push(type, resolve(text, a));
}

}  // namespace logging
