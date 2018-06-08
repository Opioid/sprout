#include "server.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "client.hpp"
#include "core/image/typed_image.hpp"
#include "core/logging/logging.hpp"
#include "message_handler.hpp"

namespace server {

Server::Server(int2 dimensions, Message_handler& message_handler)
    : srgb_(dimensions), message_handler_(message_handler) {}

Server::~Server() {
    for (Client* c : clients_) {
        delete c;
    }

    net::Socket::release();
}

void Server::run() {
    if (!net::Socket::init()) {
        logging::error("Could not start socket environment.");
        return;
    }

    accept_socket_ = net::Socket("8080");

    if (!accept_socket_.is_valid()) {
        logging::error("Could not establish accept socket.");
        return;
    }

    shutdown_ = false;

    accept_thread_ = std::thread(&Server::accept_loop, this);
}

void Server::shutdown() {
    shutdown_ = true;

    accept_socket_.cancel_blocking_accept();

    accept_thread_.join();

    accept_socket_.close();

    for (Client* c : clients_) {
        c->shutdown();
    }
}

void Server::write(const image::Float4& image, uint32_t frame, thread::Pool& pool) {
    auto const d = image.description().dimensions;
    pool.run_range([this, &image](uint32_t /*id*/, int32_t begin,
                                  int32_t end) { srgb_.to_sRGB(image, begin, end); },
                   0, d[0] * d[1]);

    size_t buffer_len = d[0] * d[1] * sizeof(byte4);

    std::string message;

    for (auto c = clients_.begin(); c != clients_.end();) {
        Client* client = *c;

        // Here we are assuming that the client disconnected if the send fails.
        // The nice solution probably is listening for a close message.

        if (0 == frame) {
            client->send(message_handler_.iteration());
        }

        if (!client->send(reinterpret_cast<const char*>(srgb_.data()), buffer_len)) {
            client->shutdown();
            delete client;
            client = nullptr;
            c      = clients_.erase(c);
        } else {
            ++c;
        }

        // If the client is still active we can process any messages that queued up
        if (client) {
            while (client->pop_message(message)) {
                message_handler_.handle(message);
            }
        }
    }
}

void Server::accept_loop() {
    accept_socket_.listen(10);

    for (;;) {
        net::Socket connection_socket = accept_socket_.accept();

        if (!connection_socket.is_valid()) {
            if (shutdown_) {
                return;
            } else {
                continue;
            }
        }

        Client* client = new Client(connection_socket);

        if (!client->run(message_handler_.introduction())) {
            delete client;
            continue;
        }

        clients_.push_back(client);
    }
}

}  // namespace server
