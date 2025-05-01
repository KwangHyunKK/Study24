#ifndef _DEMO_FAST_TCP_SRC_TCPSERVER_CPP_
#define _DEMO_FAST_TCP_SRC_TCPSERVER_CPP_

#include "../inc/tcpserver.hpp"

namespace Demo
{
    namespace Fast_Tcp
    {
        void TcpServer::start()
        {
            server_sock_ = socket(AF_INET, SOCK_STREAM, 0);
            assert(server_sock_ >= 0);

            sockaddr_in addr {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port_);

            int opt = 1;
            setsockopt(server_sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            assert(bind(server_sock_, (sockaddr*)&addr, sizeof(addr)) >= 0);
            assert(listen(server_sock_, 128) >= 0);

            running_ = true;

            for(int i=0;i<MAX_WORKER_THREADS;++i)
            {
                worker_threads_.emplace_back(&TcpServer::worker_loop, this);
            }

            std::thread(&TCPServer::accept_loop, this).detach();
        } // start

        void TcpServer::stop()
        {
            running_ = false;
            close(server_sock_);
            queue_cv_.notify_all();
            for(auto& th : worker_threads_)
            {
                if(th.joinable())th.join();
            }
        } // stop

        void TcpServer::accept_loop()
        {
            while(running_)
            {
                sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int client_sock = accept(server_sock_, (sockaddr*)&client_addr, &len);

                if(client_sock >= 0)
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    connection_queue_.push(client_sock);
                    queue_cv_.notify_one();
                }
            }
        } // accept_loop

        void TcpServer::worker_loop()
        {
            while(running_)
            {
                int client_sock = -1;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    queue_cv_.wait(lock, [this] {return !connection_queue_.empty() || !running_; });

                    if(!running)break;
                    client_sock = connection_queue_.front();
                    connection_queue_.pop();
                }

                char buffer[BUFFER_SIZE];
                while(running_)
                {
                    int bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);
                    if(bytes <= 0)break;

                    send(client_sock, buffer, bytes, 0); // Echo
                }

                close(client_sock);
            }
        } // worker_loop
    } // namespace Fast_Tcp
} // namespace Demo

#endif // _DEMO_FAST_TCP_SRC_TCPSERVER_CPP_