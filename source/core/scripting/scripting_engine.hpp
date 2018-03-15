#ifndef SU_CORE_SCRIPTING_ENGINE_HPP
#define SU_CORE_SCRIPTING_ENGINE_HPP

#include <string>

using duk_context = struct duk_hthread;

namespace scripting {

class Engine {

public:

	Engine();
	~Engine();

	void eval(const std::string& text) const;

	duk_context* context() const;

private:

	duk_context* context_;
};

}

#endif
