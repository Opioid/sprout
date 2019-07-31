#include "options.hpp"
#include <cctype>
#include <string_view>
#include "core/logging/logging.hpp"

namespace it::options {

static bool handle_all(std::string const&, std::string const& parameter, Options& result) noexcept;

static bool handle(std::string const&, std::string const& parameter, Options& result) noexcept;

static bool is_parameter(std::string_view text) noexcept;

static void help() noexcept;

Options parse(int argc, char* argv[]) noexcept {
    Options result;

    if (1 == argc) {
        help();
        return result;
    }

    for (int32_t i = 1; i < argc;) {
        std::string const command = std::string(argv[i]).substr(1);

        int32_t j = i + 1;
        for (;; ++j) {
            if (j < argc && is_parameter(argv[j])) {
                handle_all(command, argv[j], result);
            } else {
                if (j == i + 1) {
                    handle_all(command, "", result);
                }

                break;
            }
        }

        i = j;
    }

    return result;
}

bool handle_all(std::string const& command, std::string const& parameter,
                Options& result) noexcept {
    if (command[0] == '-') {
        return handle(command.substr(1), parameter, result);
    }

    for (size_t i = 0, len = command.size(); i < len; ++i) {
        if (!handle(command.substr(i, i + 1), parameter, result)) {
            return false;
        }
    }

    return true;
}

bool handle(std::string const& command, std::string const& parameter, Options& result) noexcept {
    if ("help" == command || "h" == command) {
        help();
    } else if ("diff" == command || "d" == command) {
        result.diff = true;
    } else if ("clamp" == command) {
        result.clamp = std::stof(parameter.data());
    } else if ("clip" == command) {
        result.clip = std::stof(parameter.data());
    } else if ("image" == command || "i" == command) {
        result.images.push_back(parameter);
    } else if ("threads" == command || "t" == command) {
        result.threads = std::atoi(parameter.data());
    } else {
        logging::warning("Option %S does not exist.", command);
    }

    return true;
}

bool is_parameter(std::string_view text) noexcept {
    if (text.size() <= 1) {
        return true;
    }

    if (text[0] == '-') {
        if (text[1] == '-') {
            return false;
        }

        for (size_t i = 1, len = text.size(); i < len; ++i) {
            if (!std::isdigit(text[i])) {
                return false;
            }
        }
    }

    return true;
}

void help() noexcept {
    static std::string const text =
        R"(it is a image tool
Usage:
  it [OPTION...]

  -h, --help            Print help.
  -d, --diff            Compute the difference between the first
                        and subsequent images.
                        This is the default behavior.
      --clamp    float  Clamp to this value.
      --clip     float  Clip above this value.
  -i, --image    file   Path of an image.
                        The first image is considered the reference,
                        if multiple images are specified.
  -t, --threads  int    Specifies the number of threads used by di.
                        0 creates one thread for each logical CPU.
                        -x creates a number of threads equal to the
                        number of logical CPUs minus x.
                        The default value is 0.)";

    logging::info(text);
}

}  // namespace it::options
