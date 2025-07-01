#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include <netinet/in.h>
#include <unordered_map>
#include <vector>

namespace Y25M7
{
    namespace D250701
    {
        namespace epoll
            // The event will be called when socket status becomes "readable" or "writable"
            // In this moment, developer uses ``read()`` or ``write()`` to execute data.
        {

            int set_nonblock(int fd)
            {
                int flags = fcntl(fd, F_GETFL, 0);
                return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            }
            namespace SimpleExample
            {
                int Run1()
                {
                    int listenSock = socket(AF_INET, SOCK_STREAM, 0);
                    // binding, listening
                    set_nonblock(listenSock);

                    int epfd = epoll_create1(0);
                    struct epoll_event ev, events[10];
                    ev.events = EPOLLIN;
                    ev.data.fd = listenSock;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, listenSock, &ev);

                    while (true)
                    {
                        int nfds = epoll_wait(epfd, events, 10, -1);
                        for (int i = 0; i < nfds; ++i)
                        {
                            if (events[i].data.fd == listenSock)
                            {
                                int clientSock = accept(listenSock, NULL, NULL);
                                set_nonblock(clientSock);
                                ev.events = EPOLLIN | EPOLLET; // Edge Triggered
                                ev.data.fd = clientSock;
                                epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &ev);
                            }
                            else
                            {
                                char buf[1024];
                                int len = read(events[i].data.fd, buf, sizeof(buf));
                                if (len > 0)
                                {
                                    std::cout << "Received: " << std::string(buf, len) << std::endl;
                                }
                                else
                                {
                                    close(events[i].data.fd);
                                }
                            }
                        }
                    }
                }
            } // SimpleExample namespace

            namespace EchoServer
            {
                struct Session
                {
                    int fd_;                // socket fd
                    std::string read_buf_;  // input_buffer
                    std::string write_buf_; // output_buffer
                    // Need to append game status or player status
                } // Session struct

                int Run1()
                {
                    // 1. Create and configure listen file descriptor
                    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
                    sockaddr_in addr = {0};
                    addr.sin_family = AF_INET;
                    addr.sin_port = htons(4000);
                    addr.sin_addr.s_addr = INADDR_ANY;
                    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
                    listen(listen_fd, 1000);

                    set_nonblock(listen_fd);

                    // 2. Create epoll object
                    int epfd = epoll_create1(0);

                    // epoll_event ev -> to use epoll_ctl()
                    epoll_event ev, events[64];
                    ev.events = EPOLLIN;
                    ev.data.fd = listen_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

                    std::unordered_map<int, Session> sessions;

                    while(true)
                    {
                        int n = epoll_wait(epfd, events, 64, -1);
                        for(int i=0;i<n;++i)
                        {
                            int fd = events[i].data.fd;
                            if(fd == listen_fd)
                            {
                                // new connection
                                int client_fd = accept(listen_fd, nullptr, nullptr);
                                set_nonblock(client_fd);
                                ev.events = EPOLLIN | EPOLLET;
                                ev.data.fd = client_fd;
                                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                                sessions[client_fd] = Session{client_fd};
                            }
                            else if(events[i].events & EPOLLIN)
                            {
                                // data read event
                                char buf[4096];
                                int len = read(fd, buf, sizeof(buf));
                                if(len <= 0)
                                {
                                    // termination
                                    close(fd);
                                    sessions.erase(fd);
                                }
                                else
                                {
                                    // game packet parsing and execution
                                    sessions[fd].read_buf.append(buf, len);

                                    // example : echo response
                                    // Set the response data.
                                    sessions[fd].write_buf += sessions[fd].read_buf;
                                    sessions[fd].read_buf.clear();

                                    // response with write function.
                                    if(!session[fd].write_buf.empty())
                                    {
                                        int sent = write(fd, s.write_buf.data(), s.write_buf.size());
                                        if(sent >= 0)
                                        {
                                            session[fd].write_buf.erase(0, sent);
                                        }
                                    }
                                    
                                    if(!session[fd].write_buf.empty()) // There is more data which have to send.
                                    {
                                        // EPOLLOUT registration (Monitoring write event)
                                        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                                        ev.data.fd = fd;
                                        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
                                    }
                                }
                            }
                            else if(events[i].events & EPOLLOUT)
                            {
                                // data write event
                                Session& s = sessions[fd];
                                if(!s.write_buf.empty())
                                {
                                    int len = write(fd, s.write_buf.data(), s.write_buf.size());
                                    s.write_buf.erase(0, len);
                                }
                                // if write buffer is empty -> EPOLLOUT exit
                                if(s.write_buf.empty())
                                {
                                    ev.events = EPOLLIN | EPOLLET;
                                    ev.data.fd = fd;
                                    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
                                }
                            }
                        }
                    }
                }
            }
        } // epoll namespace
    } // D250701 namespace
} // Y25M7 namespace