#ifndef _DEMO_FAST_TCP_INC_TCPCLIENT_HPP_
#define _DEMO_FAST_TCP_INC_TCPCLIENT_HPP_

#include "common.h"

namespace Demo
{
    namespace Fast_Tcp
    {
        class TcpClient
        {
        public:
            TcpClient(const std::string& server_ip, int port)
                : server_ip_(server_ip), port_(port) {}

            bool connect_server();
            void send_loop(int seconds);

        private:
            std::string server_ip_;
            int port_;
            int sock_;
        }; // TcpClient class
    } // namespace Fast_Tcp
} // namespace Demo

#endif // _DEMO_FAST_TCP_INC_TCPCLIENT_HPP_