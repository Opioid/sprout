#ifndef SU_SERVER_CLIENT_HPP
#define SU_SERVER_CLIENT_HPP

#include <deque>
#include <thread>
#include "websocket.hpp"

namespace server {

class Client {
  public:
    Client(net::Socket socket);
    ~Client();

    bool run(std::string const& introduction);
    void shutdown();

    bool send(std::string const& text);
    bool send(char const* data, size_t size);

    bool pop_message(std::string& message);

  private:
    void loop();

    void push_message(std::string const& message);

    Websocket websocket_;

    std::thread thread_;

    std::deque<std::string> messages_;
};

}  // namespace server

#endif
