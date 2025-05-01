#ifndef _DEMO_FAST_TCP_INC_TCPSERVER_HPP_
#define _DEMO_FAST_TCP_INC_TCPSERVER_HPP_

#include "common.h"

namespace Demo
{
    namespace Fast_Tcp
    {
        class TcpServer
        {
        public:
            TcpServer(int _port) : port_(port), running_(false) {}

            void start();
            void stop();
        private:
            void accept_loop();
            void worker_loop();

            int port_;
            int server_sock_;
            std::atomic<bool> running_;
            std::vector<std::thread> worker_threads_;
            std::queue<int> connection_queue_;
            std::mutex queue_mutex_;
            std::condition_variable queue_cv_;
        }; // TcpServer class

        class TcpServerEpoll
        {
        public:
            TcpServerEpoll(int port) : port_(port), running_(false) {}
            void start();
            void stop();

        private:
            void accept_loop();
            void worker_loop();
            void set_nonblocking(int sock);

            int port_;
            int server_sock_;
            int epoll_fd_;
            std::atomic<bool> running_;
            std::vector<std::thread> worker_threads_;
        }; // TcpServerEpoll Class
    } // namespace Fast_Tcp
} // namespace Demo
#endif // _DEMO_FAST_TCP_INC_TCPSERVER_HPP_