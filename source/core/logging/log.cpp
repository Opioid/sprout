#include "log.hpp"

namespace logging {

Log::~Log() {}

void Log::post(Type type, std::string const& text, std::string const& a) {
    auto const i = text.find("%S");

    std::string const concat = std::string(text.substr(0, i)) + "\"" + a + "\"" +
                               std::string(text.substr(i + 2));
    post(type, concat);
}

}  // namespace logging
