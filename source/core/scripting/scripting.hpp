#ifndef SU_CORE_SCRIPTING_SCRIPTING_HPP
#define SU_CORE_SCRIPTING_SCRIPTING_HPP

#include <string>

namespace scripting {

void init();
void close();

void test();

void eval(const std::string& text);

}

#endif
