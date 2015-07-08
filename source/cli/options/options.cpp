#include "options.hpp"
#include "core/logging/logging.hpp"
#include <sstream>
#include <tclap/CmdLine.h>

namespace options {

Options options_;

void init(int argc, char* argv[]) {
	try {
		TCLAP::CmdLine cmd("sprout is a global illumination renderer experiment", ' ', "0.1");

		TCLAP::UnlabeledValueArg<std::string> take_arg("take", "The take file to render.", false, "", "file path");
		cmd.add(take_arg);

		TCLAP::MultiArg<std::string> mount_args("m", "mount", "Sets the mount point of the data directory. "
															  "The default value is \"../data/\"", false, "directory path");
		cmd.add(mount_args);

//		TCLAP::ValueArg<std::string> mount_arg("m", "mount", "Sets the mount point of the data directory. "
//															 "The default value is \"../data/\"", false, "../data/", "folder path");
//		cmd.add(mount_arg);

		TCLAP::ValueArg<int> threads_arg("t", "threads", "Sets the number of threads. "
														 "0 uses all available threads. "
														 "-x uses all available threads but x. "
														 "The default value is 0.", false, 0, "integer number");
		cmd.add(threads_arg);

		cmd.parse(argc, argv);

		options_.take    = take_arg.getValue();
//		options_.mount   = mount_arg.getValue();
		options_.mounts  = mount_args.getValue();
		options_.threads = threads_arg.getValue();
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
