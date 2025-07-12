#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

constexpr int PORT = 12345;
constexpr int BUFSIZE = 1024;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage : " << argv[0] << " < Server Hostname or IP>\n";
        return 1;
    }

    const char *server_host = argv[1];
    const char *value = argv[2];

    struct addrinfo hints{}, *res, *p;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (int err = getaddrinfo(server_host, "12345", &hints, &res))
    {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
        return 1;
    }


    int sock_fd = -1;
    for(p = res; p;p = p->ai_next)
    {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sock_fd < 0)continue;
        if(connect(sock_fd, p->ai_addr, p->ai_addrlen) == 0)
        {
            std::cerr << "success to connect : " << p->ai_addr;
            break;
        }
        close(sock_fd);
        sock_fd = -1;
    }

    freeaddrinfo(res);

    if (sock_fd < 0)
    {
        perror("socket");
        return 1;
    }

    std::string message = "Hello from client " + std::string(value) + " ";
    message.append(std::to_string(0));
    size_t pos = message.find("0");
    for (int i = 1; i <= 50; ++i)
    {
        if (pos != std::string::npos)
        {
            std::string sub = std::to_string(i);
            message.replace(pos, sub.length(), sub);
        }

        std::cout << "[Client " << value << "] Send to Server : " << message << "\n";

        send(sock_fd, message.c_str(), message.size(), 0);

        char buffer[BUFSIZE];
        ssize_t n = recv(sock_fd, buffer, BUFSIZE - 1, 0);
        if (n < 0)
        {
            perror("recv");
        }
        else
        {
            buffer[n] = '\0';
            std::cout << "[Client " << value << "] Echoed from server : " << buffer << "\n";
        }
    }

    close(sock_fd);
    return 0;
}