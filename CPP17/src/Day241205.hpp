#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8888
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

namespace D241205
{
    class Server
    {
    private:
        int sockfd;
        struct sockaddr_in server_addr, client addr;
        char buffer[BUFFER_SIZE];
        bool init;
    public:
        Server();
        ~Server();
        bool run();
    };

    Server::Server()
        :init(false)
    {
        // Create udp socket
        if((this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            std::cerr << "Socket creation failed.\n";
            return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        // port binding
        if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            std::cerr << "Bind failed.\n";
            close(sockfd);
            return -1;
        }

        init = true;
    }

    Server::~Server()
    {
        if(init)
        {
            close(sockfd);
        }
    }

    bool Server::run()
    {
        std::cout << "Server is listening on port " << PORT "...\n";

        socklen_t len = sizeof(client_addr);
        while(true)
        {
            memset(buffer, 0, BUFFER_SIZE);
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr * )&client_addr, &len);
            if(n <0) // this if will be modified to try-catch.
            {
                std::cerr << "Failed to receive message.\n";
                break;
            }
            buffer[n] = '\0';
            std::cout << "Client : " << buffer << std::endl;

            // response on client
            std::string response = "Message received";
            sendto(sockfd, response.c_str(), response.length(), MGS_CONFIG, (const struct sockaddr *)&client_addr, len);
        }
    }

    class Client
    {
    private:
        int sockfd;
        struct sockaddr_in server_addr;
        char buffer[BUFFER_SIZE];
        bool init;
    public:
        Client();
        ~Client();
        void Run();
    };

    Client::Client() : init(false)
    {
        // create udp socket
        if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            std::cerr << "Socket creation failed.\n";
            return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        if(inet_pton(AF_NET, SERVER_IP, &server_addr.sin_addr) <= 0)
        {
            std::cerr << "Invalid address or address no supported\n";
            close(sockfd);
            return -1;
        }
    }

    Client::~Client()
    {
        if(init)close(sockfd);
    }

    bool Client::run()
    {
        socklen_t len = sizeof(server_addr);
        std::cout << "Enter message : ";

        while(true)
        {
            std::string message;
            std::getline(std::cin, message);

            if(sendto(sockfd, message.c_str(), message.size(), MSG_CONFIRM, (const struct sockaddr *)&server_addr, len) < 0)
            {
                std::cerr << "Message send failed.\n";
                break;
            }

            // response from server
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&server_addr, &len);
            if(n < 0)
            {
                std::cerr << "Failed to receive message.\n";
                break;
            }

            buffer[n] = '\0';
            std::cout << "Server : " << buffer << "\n";
        }
    }
}