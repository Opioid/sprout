#pragma once

#include <string>

namespace logging {

enum class Type {
	Null,
	Stdout
};

void init(Type type);
void release();

void info(const std::string& text);
void warning(const std::string& text);
void error(const std::string& text);

}
