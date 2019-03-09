#pragma once

#include <string>

namespace server {

class Message_handler {
  public:
    virtual ~Message_handler() {}

    virtual void handle(std::string const& message) = 0;

    virtual std::string introduction() const = 0;

    virtual std::string iteration() const = 0;
};

}  // namespace server
