#ifndef _DEMO_EVENT_SUBSCRIBER_COMMON_ENDPOINT_HPP_
#define _DEMO_EVENT_SUBSCRIBER_COMMON_ENDPOINT_HPP_

#include <string>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace Demo
{
    namespace Event_Subscriber
    {

        struct ClientInfo
        {
            std::string id;
            std::string ip;
            uint16_t port;
        }; // ClientInfo struct

        // 자원관리가 확실하게 되어야 한다.
        // socket을 추상화 -> socket의 multicast, unicast를 구분하고, 
        // 1개의 자원이 1개의 socket이 되도록 해야한다.
        // 결국 -> 여러 module에서 접근할 수 있는 공통의 자원이 되어야 한다.
        class Endpoint
        {
        public:
            void subscribe(const ClientInfo& client)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                clients_[client.id_] = client;
                if(clients_.size() >=3 && !is_multicast_)
                {
                    setup_multicast();
                }
            }

            void broadcast(const std::string& message)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if(is_multicast_)
                {
                    send_multicast(message);
                }
                else
                {
                    for(const auto& [id, client] : clients_)
                    {
                        send_unicast(client, message);
                    }
                }
            }
        private:
            std::unordered_map<std::string, ClientInfo> clients_;
            std::mutex mutex_;
            bool is_multicast_ = false;
            int multicast_sock_ = -1;
            struct sockaddr_in multicast_addr_{};

            void setup_multicast() {
                multicast_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
                memset(&multicast_addr_, 0, sizeof(multicast_addr_));
                multicast_addr_.sin_family = AF_INET;
                multicast_addr_.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);
                multicast_addr_.sin_port = htons(MULTICAST_PORT);
                is_multicast_ = true;
                close(multicast_sock_);
            }
        
            void send_multicast(const std::string& message) {
                sendto(multicast_sock_, message.c_str(), message.size(), 0,
                       (struct sockaddr*)&multicast_addr_, sizeof(multicast_addr_));
            }
        
            void send_unicast(const ClientInfo& client, const std::string& message) {
                int sock = socket(AF_INET, SOCK_DGRAM, 0);
                struct sockaddr_in client_addr{};
                client_addr.sin_family = AF_INET;
                client_addr.sin_port = htons(client.port);
                inet_pton(AF_INET, client.ip.c_str(), &client_addr.sin_addr);
                sendto(sock, message.c_str(), message.size(), 0,
                       (struct sockaddr*)&client_addr, sizeof(client_addr));
                close(sock);
            }
        }; // Endpoint class
    } // namespace Event_Subscriber
} // namespace Demo
#endif