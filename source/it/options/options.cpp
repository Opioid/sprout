#include "options.hpp"
#include "base/math/vector2.inl"
#include "core/logging/logging.hpp"

#include <cctype>
#include <charconv>
#include <string_view>

namespace it::options {

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
    } else if ("add" == command) {
        result.op = Options::Operator::Add;
    } else if ("average" == command || "a" == command) {
        result.op = Options::Operator::Average;
    } else if ("cat" == command || "c" == command) {
        result.op = Options::Operator::Cat;
        std::from_chars(parameter.data(), parameter.data() + parameter.size(),
                        result.concat_num_per_row);
    } else if ("diff" == command || "d" == command) {
        result.op = Options::Operator::Diff;
    } else if ("clamp" == command) {
        result.clamp = std::stof(parameter);
    } else if ("clip-lo" == command) {
        result.clip[0] = std::stof(parameter);
    } else if ("clip-hi" == command) {
        result.clip[1] = std::stof(parameter);
    } else if ("max-dif" == command) {
        result.max_dif = std::stof(parameter);
    } else if ("image" == command || "i" == command) {
        result.images.push_back(parameter);
    } else if ("out" == command || "o" == command) {
        result.outputs.push_back(parameter);
    } else if ("no-export" == command || "n" == command) {
        result.no_export = true;
    } else if ("report" == command || "r" == command) {
        result.report = parameter.empty() ? "." : parameter;
    } else if ("stats" == command || "s" == command) {
        result.statistics = parameter.empty() ? "." : parameter;
    } else if ("sub" == command) {
        result.op = Options::Operator::Sub;
    } else if ("take" == command) {
        result.take = parameter;
    } else if ("threads" == command || "t" == command) {
        std::from_chars(parameter.data(), parameter.data() + parameter.size(), result.threads);
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
        R"(it is an image tool
Usage:
  it [OPTION...]

  -h, --help                  Print help.
      --add                   Add a series of images and save as
                              a single image.
  -a, --average               Calculate the average of an image.
  -c, --cat      int?         Concatenate a series of images and save as
                              a single image.
                              Optionally specify after how many images
                              a new row should be started.
      --clamp    float        Clamp to the given value.
      --clip-lo  float        Clip below the given value.
      --clip-hi  float        Clip above the given value.
  -d, --diff                  Compute the difference between the first
                              and subsequent images.
      --max-dif  float        Override the calculated max difference
                              for coloring the difference images.
  -i, --image    file+        File name of an image.
                              For the diff operator, the first image is
                              considered the reference, if multiple
                              images are specified.
  -o, --out      file+        File name of an output image.
                              In case of diff, as many outputs as
                              images minus 1 can be specified.
                              WARNING:
                              In case of missing file names, it will pick
                              defaults that could overwrite existing files!
  -n, --no-export             Disables export of images.
  -r, --report   file?        Generate report.
                              Optionally the report can be written to a file.
  -s, --stats    file?        Generate image statistics, including histogram.
                              Optionally the stats can be written to a file.
                              If no operator is specified, it defaults to stats.
      --sub                   Subtract a series of images from the first image
                              and save as a single image.
      --take     file/string  Path of the take file to render,
                              or json-string describing the take.
  -t, --threads  int          Specifies the number of threads used by it.
                              0 creates one thread for each logical CPU.
                              -x creates as many threads as the number of
                              logical CPUs minus x.
                              The default value is 0.)";

    logging::info(text);
}

}  // namespace it::options
