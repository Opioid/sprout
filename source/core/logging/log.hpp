#pragma once

#include <string>

namespace logging {

class Log {

public:

	enum class Type {
		Info,
		Warning,
		Error,
		Verbose
	};

	virtual ~Log() {}

	virtual void post(Type type, const std::string& text) = 0;
};

}
