#pragma once

#include "log.hpp"

namespace logging {

class Stdout : public Log {
public:

	virtual void post(Type type, const std::string& text) final override;
};

}


