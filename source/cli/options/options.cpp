#include "options.hpp"
#include "core/logging/logging.hpp"

#include <cctype>
#include <charconv>
#include <string_view>

namespace options {

static bool handle_all(std::string const&, std::string const& parameter, Options& result);

static bool handle(std::string const&, std::string const& parameter, Options& result);

static bool is_parameter(std::string_view text);

static void help();

Options parse(int argc, char* argv[]) {
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

bool handle_all(std::string const& command, std::string const& parameter, Options& result) {
    if (command[0] == '-') {
        return handle(command.substr(1), parameter, result);
    }

    for (size_t i = 0, len = command.size(); i < len; ++i) {
        if (!handle(command.substr(i, 1), parameter, result)) {
            return false;
        }
    }

    return true;
}

bool handle(std::string const& command, std::string const& parameter, Options& result) {
    if ("help" == command || "h" == command) {
        help();
    } else if ("frame" == command || "f" == command) {
        std::from_chars(parameter.data(), parameter.data() + parameter.size(), result.start_frame);
    } else if ("input" == command || "i" == command) {
        result.take = parameter;
    } else if ("mount" == command || "m" == command) {
        result.mounts.push_back(parameter);
    } else if ("threads" == command || "t" == command) {
        std::from_chars(parameter.data(), parameter.data() + parameter.size(), result.threads);
    } else if ("quit" == command || "q" == command) {
        result.quit = true;
    } else if ("no-textures" == command) {
        result.no_textures = true;
    } else if ("no-tex-dwim" == command) {
        result.no_tex_dwim = true;
    } else if ("debug-material" == command) {
        result.debug_material = true;
    } else {
        logging::warning("Option %S does not exist.", command);
    }

    return true;
}

bool is_parameter(std::string_view text) {
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

void help() {
    static std::string const text =
        R"(sprout is a global illumination renderer experiment
Usage:
  sprout [OPTION...]

  -h, --help                  Print help.
  -f  --frame    int          Index of the frame to render.
                              Overrides starting_frame specified in take file.
  -i, --input    file/string  Path of the take file to render,
                              or json-string describing the take.
  -m, --mount    path+        Specifies a mount point for the data
                              directory. The default value is "../data/"
  -t, --threads  int          Specifies the number of threads used by sprout.
                              0 creates one thread for each logical CPU.
                              -x creates as many threads as the number of
                              logical CPUs minus x. The default value is 0.
  -q, --quit                  Automatically quit sprout after rendering.
      --no-textures           Disables loading of all textures
      --no-tex-dwim           Disables loading of most textures)";

    logging::info(text);
}

}  // namespace options
