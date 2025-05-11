#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CLIENT 100
#define MULICAST_ADDR "239.0.0.1"
#define MULTICAST_PORT 8888

namespace NoBuild
{
    struct ClientInfo
    {
        std::string id;
        std::string ip;
        uint16_t port;
    };

    class Endpoint
    {
    public:
        void subscribe(consts ClientInfo &client)
        {
            std::lock_guard<std : mutex> lock(mutex_);
            clients_[client.id] = client;
            if (clients_.size() >= 3 && !is_multicast_)
            {
                setup_multicast();
            }
        }

        void broadcast(const std::string &message)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (is_multicast_)
            {
                send_multicast(message);
            }
            else
            {
                for (const auto &[id, client] : clients_)
                {
                    send_unicast(client, message);
                }
            }
        }

    private:
        std::unordered_map<std::string, ClientInfo> clients_;
        std::mutex mutex_;
        std::is_multicast_ = false;
        int multicast_sock_ = -1;
        struct sockaddr_in multicast_addr_{};

        void setup_multicast()
        {
            multicast_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
            memset(&multicast_addr_, 0, sizeof(multicast_addr_));
            multicast_addr_.sin_family = AF_INET;
            multicast_addr_.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);
            multicast_addr_.sin_prot = htons(MULTICAST_PORT);
            is_multicast_ = true;
        }

        void send_multicast(const std::string& message)
        {
            sendto(multicast_sock_, message.c_str(), message.size(), 0, (struct sockaddr*)&multicast_addr_, sizeof(multicast_addr_));
        }

        void send_unicast(const ClientInfo& client, const std::string& message)
        {
            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in client_addr{};
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(client.port);
            inet_pton(AF_INET, client.ip.c_str(), &client_addr.sin_addr);
            sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            close(sock);
        }
    };

    class Server
    {
    public:
        void run()
        {
            std::thread sub_thread(&Server::subscription_thread, this);
            std::thread state_thread(&Server::state_thread, this);
            std::thread send_thread(&Server::send_thread, this);

            if(sub_thread.joinable())sub_thread.join();
            if(state_thread.joinable())state_thread.join();
            if(send_thread.joinable())send_thread.join();
        }

    private:
        Endpoint endpoint_;
        std::mutex queue_mutex_;
        std::vector<std::string> event_queue_;

        void subscription_thread()
        {
            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in serv_addr{}, cli_addr{};
            socklen_t len = sizeof(cli_addr);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = INADDR_ANY;
            serv_addr.sin_port = htons(9999);
            bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

            while(true)
            {
                char buffer[1024] = {0};
                recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, &len);
                std::string id(buffer);
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cli_addr.sin_addr, ip, INET_ADDRSTRLEN);
                uint16_t port = ntohs(cli_addr.sin_port);
                endpoint_.subscribe({id, ip, port});
            }
        }

        void state_thread()
        {
            while(true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                std::lock_guard<std::mutex> lock(queue_mutex_);
                event_queue_.emplace_back("Event triggered");
            }
        }

        void send_thread()
        {
            while(true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if(!event_queue_.empty())
                {
                    endpoint_.broadcast(event_queue_.front());
                    event_queue_.erase(event_queue_.begin());
                }
            }
        }
    };

    class Client
    {
    public:
        using Callback = std::function<void(const std::string&)>;

        void start(const std::string& id, const std::string& server_ip, Callback cb)
        {
            callback_ = cb;
            id_ = id;
            register_with_server(server_ip);
            std::thread listener(&client::listen_thread, this);
            listener.detach();
        }

    private:
        std::string id_;
        Callback callback_;

        void register_with_server(const std::string& server_ip)
        {
            int sock=socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in serv_addr{};
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(9999);
            inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr);
            sendto(sock, id_.c_str(), id_.size(), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            close(sock);
        }

        void listen_thread()
        {
            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in cli_addr{};
            cli_addr.sin_family = AF_INET;
            cli_addr.sin_addr.s_addr = INADDR_ANY;
            cli_addr.sin_port = htons(0); // OS assigns port
            bind(sock, (struct sockaddr*)&cli_addr, sizeof(cli_addr));

            while(ture)
            {
                char buffer[1024] = {0};
                recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
                callback_(std::string(buffer));
            }
        }
    }
}