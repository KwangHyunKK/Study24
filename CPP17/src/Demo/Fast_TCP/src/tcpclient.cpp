#ifndef _DEMO_FAST_TCP_SRC_TCPCLIENT_CPP_
#define _DEMO_FAST_TCP_SRC_TCPCLIENT_CPP_

#include "../inc/tcpclient.hpp"

namespace Demo
{
    namespace Fast_Tcp
    {
        bool TcpClient::connect_server()
        {
            sock_ = socket(AF_INET, SOCK_STREAM, 0);
            if(sock < 0)return false;

            sockadrr_in addr {};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port_);
            inet_pton(AF_INET, server_ip_.c_str(), &addr.sin_addr);

            return connect(sock_, (sockaddr*)&addr, sizeof(addr)) == 0;
        } // connect_server

        void Tcpclient::send_loop(int seconds)
        {
            auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
            char buffer[BUFFER_SIZE];
            memset(buffer, 'A', sizeof(buffer));

            while(std::chrono::steady_clock::now() < end_time)
            {
                send(sock_, buffer, sizeof(buffer), 0);
                int val = recv(sock_, buffer, sizeof(buffer), 0);
            }

            close(sock_);
        } // send_loop
    } // namespace Fast_Tcp
} // namespace Demo
#endif // _DEMO_FAST_TCP_SRC_TCPCLIENT_CPP_