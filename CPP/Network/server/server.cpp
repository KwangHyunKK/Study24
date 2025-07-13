#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int PORT = 12345;
constexpr int BUFSIZE = 1024;

int main(int argc, char* argv[])
{
    // create listen file descriptor
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스 바인딩
    serv_addr.sin_port = htons(PORT);

    int opt = 1;
    // set socket option : REUSEADDR
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // bind
    if(bind(listen_fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0)
    {
        perror("bind");
        close(listen_fd);
        return 1;
    }


    // listen
    if (listen(listen_fd, 5) < 0)
    {
        perror("Listen");
        close(listen_fd);
        return 1;
    }

    std::cout << "Echo server listening on port " << PORT << "...\n";

    while(true)
    {
        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);
        // accept
        int conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&cli_addr), &cli_len);
        if(conn_fd < 0)
        {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, sizeof(client_ip));
        std::cout << "Connected from " << client_ip << ":" << ntohs(cli_addr.sin_port) << "\n";

        char buffer[BUFSIZE];
        ssize_t n;

        while((n = recv(conn_fd, buffer, BUFSIZE - 1, 0)) > 0)
        {
            buffer[n] = '\0';
            std::cout << "Received : " << buffer << "\n";
            send(conn_fd, buffer, n, 0);
        }

        if(n < 0)
        {
            perror("recv");
        }

        close(conn_fd);
        std::cout << "Connected Closed.\n";
    }

    close(listen_fd);
    return 0;
}