#include "options.hpp"
#include "core/logging/logging.hpp"
#include "cxxopts/cxxopts.hpp"
#include <sstream>

namespace options {

Options parse(int argc, const char* argv[]) {
	Options result;

	try {
		cxxopts::Options options("sprout", "sprout is a global illumination renderer experiment");

		options.add_options()
			("h, help", "Print help.")

			("i, input",
			 "Path of the take file to render or json string describing the take.",
			 cxxopts::value<std::string>(result.take), "json file/string")

			("m, mount",
			 "Specifies a mount point for the data directory. "
			 "The default value is \"../data/\"",
			 cxxopts::value<std::vector<std::string>>(result.mounts), "directory path")

			("t, threads",
			 "Specifies the number of threads used by sprout. "
			 "0 creates one thread for each logical CPU. "
			 "-x creates a number of threads equal to the number of logical CPUs minus x. "
			 "The default value is 0.",
			 cxxopts::value<int>(result.threads), "integer")

			("p, progressive",
			 "Starts sprout in progressive mode.",
			 cxxopts::value<bool>(result.progressive))

			("no-textures",
			 "Disables loading of all textures.",
			 cxxopts::value<bool>(result.no_textures))

			("v, verbose",
			 "Enables verbose logging.",
			 cxxopts::value<bool>(result.verbose))
		;

		const int initial_argc = argc;

		const auto parsed = options.parse(argc, argv);

		if (1 == initial_argc || parsed.count("help")) {
			std::stringstream stream;
			stream << options.help({"", "Group"});
			logging::info(stream.str());
		}
	} catch (const cxxopts::OptionException& e) {
		std::stringstream stream;
		stream << "Parsing options: " << e.what();
		logging::error(stream.str());
	}

	return result;
}

}
