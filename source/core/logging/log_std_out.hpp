#ifndef SU_CORE_LOGGING_STD_OUT_HPP
#define SU_CORE_LOGGING_STD_OUT_HPP

#include "log.hpp"

namespace logging {

class Std_out : public Log {

public:

	virtual void post(Type type, const std::string& text) override final;
};

}

#endif
