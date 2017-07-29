#include "options.hpp"
#include "core/logging/logging.hpp"
#include <sstream>
#include <tclap/CmdLine.h>

namespace options {

Options options_;

void init(int argc, char* argv[]) {
	try {
		TCLAP::CmdLine cmd("sprout is a global illumination renderer experiment", ' ', "0.1");

		TCLAP::UnlabeledValueArg<std::string> take_arg(
					"take", "Path of the take file to render or json string describing the take.",
					true, "", "file path/json string");
		cmd.add(take_arg);

		TCLAP::MultiArg<std::string> mount_args(
					"m", "mount",
					"Specifies a mount point for the data directory. "
					"The default value is \"../data/\"",
					false, "directory path");
		cmd.add(mount_args);

		TCLAP::ValueArg<int> threads_arg(
					"t", "threads",
					"Specifies the number of threads used by sprout. "
					"0 creates one thread for each logical CPU. "
					"-x creates a number of threads equal to the number of logical CPUs minus x. "
					"The default value is 0.",
					false, 0, "integer number");
		cmd.add(threads_arg);

		TCLAP::SwitchArg progressive_arg(
					"p", "progressive",
					"Starts sprout in progressive mode.",
					false);
		cmd.add(progressive_arg);

		TCLAP::SwitchArg no_textures_arg(
					"", "no-textures",
					"Disables loading of all textures.",
					false);
		cmd.add(no_textures_arg);

		TCLAP::SwitchArg verbose_arg(
					"v", "verbose",
					"Enables verbose logging.",
					false);
		cmd.add(verbose_arg);

		cmd.parse(argc, argv);

		options_.take		 = take_arg.getValue();
		options_.mounts		 = mount_args.getValue();
		options_.threads	 = threads_arg.getValue();
		options_.progressive = progressive_arg.getValue();
		options_.no_textures = no_textures_arg.getValue();
		options_.verbose     = verbose_arg.getValue();
	} catch (TCLAP::ArgException& e) {
		std::stringstream stream;
		stream << e.error() << " for arg " << e.argId();
		logging::error(stream.str());
	}
}

const Options& options() {
	return options_;
}

}
