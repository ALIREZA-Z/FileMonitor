#include <zmq.hpp>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <cstdlib>  // for system()

// ---------------- Utility: split string ----------------
std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

// ---------------- Utility: run robocopy ----------------
void runRobocopy(const std::string& src, const std::string& dest) {
    std::string cmd = "robocopy \"" + src + "\" \"" + dest + "\" /MIR /NFL /NDL /NJH /NJS /nc /ns /np";
    std::cout << "[Server] Running: " << cmd << std::endl;
    system(cmd.c_str());
}

// ---------------- Timer class with callback ----------------
class Timer {
public:
    Timer() : running(false) {}

    void start(int delay_ms, std::function<void()> callback) {
        stop();
        running = true;
        worker = std::thread([=]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            if (running) {
                callback();
            }
        });
        worker.detach();
    }

    void stop() {
        running = false;
    }

private:
    std::atomic<bool> running;
    std::thread worker;
};

// ---------------- Main Server ----------------
int main() {
    try {
        zmq::context_t context(1);
        zmq::socket_t socket(context, zmq::socket_type::pull);
        socket.bind("tcp://*:5555");

        std::cout << "[Server] Listening on tcp://*:5555\n";

        // Map clientID -> (src, dest)
        std::map<std::string, std::pair<std::string, std::string>> clients;

        // Map clientID -> timer
        std::map<std::string, Timer> timers;

        const int coldDelayMs = 5000; // 5 seconds idle before robocopy

        while (true) {
            zmq::message_t message;
            socket.recv(message, zmq::recv_flags::none);

            std::string msg(static_cast<char*>(message.data()), message.size());
            auto parts = split(msg, '|');

            if (parts.size() == 3 && parts[1].rfind("SRC:", 0) == 0 && parts[2].rfind("DST:", 0) == 0) {
                // Registration
                std::string clientID = parts[0];
                std::string src = parts[1].substr(4);
                std::string dest = parts[2].substr(4);
                clients[clientID] = {src, dest};
                std::cout << "[Server] Registered client: " << clientID
                          << " src: " << src << " dest: " << dest << "\n";

            } else if (parts.size() == 2) {
                // Normal file action
                std::string clientID = parts[0];
                std::string actionMsg = parts[1];

                auto it = clients.find(clientID);
                if (it != clients.end()) {
                    std::cout << "[Server][" << clientID << "] "
                              << actionMsg << " (src: " << it->second.first
                              << " dest: " << it->second.second << ")\n";

                    // Reset timer
                    timers[clientID].start(coldDelayMs, [clientID, &clients]() {
                        auto it2 = clients.find(clientID);
                        if (it2 != clients.end()) {
                            runRobocopy(it2->second.first, it2->second.second);
                        }
                    });

                } else {
                    std::cout << "[Server] Unknown client: " << clientID
                              << " message: " << actionMsg << "\n";
                }
            } else {
                std::cout << "[Server] Invalid message: " << msg << "\n";
            }
        }

    } catch (const zmq::error_t& ex) {
        std::cerr << "ZeroMQ error: " << ex.what() << std::endl;
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
