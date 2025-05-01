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

        void TcpServerEpoll::set_nonblocking(int sock)
        {
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags | O_ONBLOCK);
        }

        void TcpServerEpoll::start()
        {
            server_sock_ = socket(AF_INET, SOCK_STREAM, 0);a
            assert(server_sock_ >= 0);
            sockaddr_in addr {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port_);

            int opt = 1;
            setsockopt(server_sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            assert(bind(server_sock_, (sockaddr*)&addr,sizeof(addr)) >= 0);
            assert(listen(server_sock, 128) >= 0);
            
            set_nonblocking(server_sock_);

            epoll_fd_ = epoll_create1(0);
            assert(epoll_fd_ >= 0);

            epoll_event ev {};
            ev.events = EPOLLIN;
            ev.data.fd = server_sock_;
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_sock_, &ev);

            running_ = true;

            // worker thread start
            for(int i=0;i<MAX_WORKER_THREADS;++i)
            {
                worker_threads_.emplace_back(&TcpServerEpoll::worker_loop, this);
            }
        } // start

        void TcpServerEpoll::stop()
        {
            running_ = false;
            close(server_sock_);
            close(epoll_fd_);

            for(auto& th:worker_threads_)
            {
                if(th.joinable())th.join();
            }
        } // stop

        void TcpServerEpoll::worker_loop()
        {
            constexpr int MAX_EVENTS = 64;
            epoll_event events[MAX_EVENTS];

            while(running_)
            {
                int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, 1000); // timeout 1 second
                for(int i=0;i<n;++i)
                {
                    int fd = events[i].data.fd;

                    if(fd == server_sock_)
                    {
                        // new connection
                        while(true)
                        {
                            sockaddr_in client_addr;
                            socklen_t len = sizeof(client_addr);
                            int client_sock = accept(server_sock_, (sockaddr*)&client_addr, &len);
                            if(client_sock < 0)break;

                            set_nonblocking(client_sock);

                            epoll_event ev{};
                            ev.events = EPOLLIN | EPOLLET; // Edge triggered
                            ev.data.fd = client_sock;
                            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_sock, &ev);
                        }
                    }
                    else
                    {
                        // Data receive
                        char buffer[BUFFER_SIZE];
                        while(true)
                        {
                            int bytes = recv(fd, buffer, sizeof(buffer), 0);
                            if(bytes <= 0)
                            {
                                if(errno !+ EAGAIN && errno != EWOULDBLOCK)
                                {
                                    close(fd);
                                }
                                break;
                            }
                            send(fd, buffer, bytes, 0); // echo
                        }
                    }
                }
            }
        } // worker_loop

    } // namespace Fast_Tcp
} // namespace Demo

#endif // _DEMO_FAST_TCP_SRC_TCPSERVER_CPP_