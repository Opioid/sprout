#ifndef SU_SERVER_SERVER_HPP
#define SU_SERVER_SERVER_HPP

#include "base/net/socket.hpp"
#include "core/exporting/exporting_sink.hpp"
#include "image/encoding/encoding_srgb.hpp"

#include <list>
#include <thread>

namespace server {

class Client;
class Message_handler;

class Server : public exporting::Sink {
  public:
    Server(Message_handler& message_handler) noexcept;

    ~Server() noexcept override final;

    void run() noexcept;

    void shutdown() noexcept;

    void write(image::Float4 const& image, uint32_t frame,
               thread::Pool& threads) noexcept override final;

  private:
    void accept_loop();

    image::encoding::Srgb_alpha srgb_;

    Message_handler& message_handler_;

    std::thread accept_thread_;

    net::Socket accept_socket_;

    bool shutdown_;

    std::list<Client*> clients_;
};

}  // namespace server

#endif
