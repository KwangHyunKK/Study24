#ifndef _DEMO_EVENT_SUBSCRIBER_CLIENT_SERVER_HPP_
#define _DEMO_EVENT_SUBSCRIBER_CLIENT_SERVER_HPP_

namespace Demo
{
    namespace Event_Subscriber
    {
        class Client
        {
        public:
            using Callback = std::function<void(const std::string&)>;

            void start(const std::string& id, const std::string& server_ip, Callback cb)
            {
                callback_ = cb;
                id_ = id;
                register_with_server(server_ip);
                std::thread listener(&Client::listen_thread, this);
                listener.detach();
            }
        private:
            std::string id_;
            Callback callback_;

            void register_with_server(const std::string& server_ip)
            {
                int sock = socket(AF_INET, SOCK_DGRAM, 0);
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
                struct sockaddr_int cli_addr{};
                cli_addr.sin_family = AF_INET;
                cli_addr.sin_addr.s_addr = INADDR_ANY;
                cli_addr.sin_port = htons(0);
                bind(sock, (struct sockaddr*)&cli_addr, sizeof(cli_addr));

                while(true)
                {
                    char buffer[1024] = {0};
                    recvfrom(sock, buffer,sizeof(buffer), 0, nullptr, nullptr);
                    callback_(std::string(buffer));
                }
            }
        }; // Client class 
    } // namespace Event_Subscriber
} // namespace Demo
#endif