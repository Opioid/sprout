#pragma once

#include "log.hpp"

namespace logging {

class Std_out : public Log {

public:

	virtual void post(Type type, const std::string& text) override final;
};

}


