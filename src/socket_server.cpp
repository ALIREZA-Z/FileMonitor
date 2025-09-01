#include <asio.hpp>
#include <asio/local/stream_protocol.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>

#include "common/socket_utils.h"

using asio::local::stream_protocol;

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(stream_protocol::socket socket)
        : socket_(std::move(socket)) {}

    void start() { doRead(); }

private:
    void doRead() {
        auto self(shared_from_this());
        socket_.async_read_some(
            asio::buffer(data_),
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string msg(data_.data(), length);
                    std::cout << "Received: " << msg << "\n";
                    doRead();
                }
            }
        );
    }

    stream_protocol::socket socket_;
    std::array<char, 256> data_;
};

class Server {
public:
    Server(asio::io_context& io, const std::string& socketPath)
        : acceptor_(io, stream_protocol::endpoint(socketPath)) {
        doAccept();
    }

private:
    void doAccept() {
        acceptor_.async_accept(
            [this](std::error_code ec, stream_protocol::socket socket) {
                if (!ec) std::make_shared<Session>(std::move(socket))->start();
                doAccept();
            }
        );
    }

    stream_protocol::acceptor acceptor_;
};

int main() {
    try {
        asio::io_context io;

        std::string socketPath = getTempSocketPath();
        // remove old socket if exists
        DeleteFileA(socketPath.c_str());

        Server server(io, socketPath);
        std::cout << "Server listening on: " << socketPath << "\n";

        // Run IO context with thread pool
        unsigned int threads = std::thread::hardware_concurrency();
        threads = threads > 0 ? threads : 2;
        std::vector<std::thread> pool;
        for (unsigned int i = 0; i < threads; ++i)
            pool.emplace_back([&io]{ io.run(); });
        for (auto& t : pool) t.join();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
