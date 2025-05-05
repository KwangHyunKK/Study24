#ifndef _DEMO_EVENT_SUBSCRIBER_SERVER_SERVER_HPP_
#define _DEMO_EVENT_SUBSCRIBER_SERVER_SERVER_HPP_

#include <thread>
#include <mutex>
#include <vector>


#include "../Common/endpoint.hpp"

namespace Demo
{
    namespace Event_Subscriber
    {
        class Server
        {
        public:
            void run()
            {
                std::thread sub_thread(&Server::subcription_thread, this);
                std::thread state_thread(&Server::state_thread, this);
                std::thread send_thread(&Server::send_thread, this);

                sub_thread.join();
                state_thread.join();
                send_thread.join();
            }

        private:
            Endpoint endpoint_;
            std::mutex queue_mutex_;
            std::vector<std::string> event_queue_;

            void subscription_thread()
            {
                int sock = socket(AF_INET, SOCK_DGRAM, 0); // 해당 socket의 자원 관리가 명시적으로 이루어지지 않고 있다.
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

                close(sock);
            }

            // The state_thread and send thread shall be upgrade.
            // sleep -> cv
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
                    std::this_thread;:sleep_for(std::chrono::seconds(1));
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    if(!event_queue_.empty())
                    {
                        endpoint_.broadcast(event_queue_.front());
                        endpoint_.erase(event_queue_.begin());
                    }
                }
            }
        }; // Server class
    } // namespace Event_Subscriber
} // namespace Demo
#endif