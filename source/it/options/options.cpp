#include "options.hpp"
#include <cctype>
#include <string_view>
#include "base/math/vector2.inl"
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
        if (!handle(command.substr(i, 1), parameter, result)) {
            return false;
        }
    }

    return true;
}

bool handle(std::string const& command, std::string const& parameter, Options& result) noexcept {
    if ("help" == command || "h" == command) {
        help();
    } else if ("diff" == command || "d" == command) {
        result.op = Options::Operator::Diff;
    } else if ("cat" == command || "c" == command) {
        result.op                 = Options::Operator::Cat;
        result.concat_num_per_row = static_cast<uint32_t>(std::atoi(parameter.data()));
    } else if ("clamp" == command) {
        result.clamp = std::stof(parameter.data());
    } else if ("clip-lo" == command) {
        result.clip[0] = std::stof(parameter.data());
    } else if ("clip-hi" == command) {
        result.clip[1] = std::stof(parameter.data());
    } else if ("max-dif" == command) {
        result.max_dif = std::stof(parameter.data());
    } else if ("image" == command || "i" == command) {
        result.images.push_back(parameter);
    } else if ("out" == command || "o" == command) {
        result.outputs.push_back(parameter);
    } else if ("no-export" == command || "n" == command) {
        result.no_export = true;
    } else if ("report" == command || "r" == command) {
        result.report = parameter.empty() ? "." : parameter;
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
  -c, --cat      int?   Concatenate multiple images and save as
                        a single image.
                        Optionally specify after how many images
                        a new row should be started.
      --clamp    float  Clamp to the given value.
      --clip-lo  float  Clip below the given value.
      --clip-hi  float  Clip above the given value.
	  --max-dif  float  Override the calculated max difference
						for coloring the difference images.
  -i, --image    file+  File name of an image.
                        For the diff operator, the first image is
                        considered the reference, if multiple
                        images are specified.
  -o, --out      file+  File name of an output image.
                        In case of diff, as many outputs as
                        images minus 1 can be specified.
                        WARNING:
                        In case of missing file names, it will pick
                        defaults that could overwrite existing files!
  -n, --no-export       Disables export of images.
  -r, --report   file?  Generate report.
                        Optionally the report can be written to a file,
                        instead of being displayed in the terminal.
  -t, --threads  int    Specifies the number of threads used by it.
                        0 creates one thread for each logical CPU.
                        -x creates as many threads as the number of
                        logical CPUs minus x.
                        The default value is 0.)";

    logging::info(text);
}

}  // namespace it::options
