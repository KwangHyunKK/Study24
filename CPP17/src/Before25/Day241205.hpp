#include <vector>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <unordered_set>

#define PORT 8888
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

namespace D241205
{
    class Server
    {
    private:
        std::vector<int> sockfd; // use vector for multi ports ports : socket 1:1 mapping
        std::unordered_set<int> boundPorts;
        struct sockaddr_in server_addr, client addr;
        char buffer[BUFFER_SIZE];
        std::vector<int> ports; // In this case, client_addr is a variable. In other case, (client_addr, ports) shall be mapeped.
        bool init;

        int bindToAvailablePort()
        {
            int reV = -1;
            for(const auto& port : ports)
            {
                auto iter = boundPorts.find(port);
                // If port has been bound, it doesn't have to work.
                if(iter == boundPorts.end()) 
                {
                    int localsock = socket(AF_INET, SOCK_DGRAM, 0);
                    if(localsock < 0)
                    {
                        std::cerr << "Failed to create socket for port " << port << "\n";
                        reV = -1;
                        continue;
                    }

                    server_addr.sin_family = AF_INET;
                    server_addr.sin_addr.s_addr = INADDR_ANY;
                    server_addr.sin_port = htons(port);

                    if(bind(localsock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                    {
                        std::cerr << "Bind failed.\n";
                        close(localsock);
                        reV = -1;
                    }
                    else
                    {
                        boundPorts.insert(port);
                        sockfd.push_back(localsock);
                        reV = 0;
                    }
                }

                if(reV == -1)return -1;
            }
        }
    public:
        Server(const std::vector<int>& inputs);
        ~Server();
        bool run(){}

        void receiveMessages();
        void sendMessages();
    };

    Server::Server(const std::vector<int>& inputs)
        :ports(inputs), init(false)
    {
        if(bindToAvailablePort() != -1)init = true;
    }

    Server::~Server()
    {
        if(init)
        {
            for(const auto& sock : sockfd)
            {
                close(sockfd);
            }

            sockfd.clear();
            boundPorts.clear();
        }
    }

    // bool Server::run()
    // {
    //     if(!init)return false;
    //     std::cout << "Server is listening on port " << PORT "...\n";

    //     socklen_t len = sizeof(client_addr);
    //     while(true)
    //     {
    //         memset(buffer, 0, BUFFER_SIZE);
    //         int n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr * )&client_addr, &len);
    //         if(n <0) // this if will be modified to try-catch.
    //         {
    //             std::cerr << "Failed to receive message.\n";
    //             break;
    //         }
    //         buffer[n] = '\0';
    //         std::cout << "Client : " << buffer << std::endl;

    //         // response on client
    //         std::string response = "Message received";
    //         sendto(sockfd, response.c_str(), response.length(), MGS_CONFIG, (const struct sockaddr *)&client_addr, len);
    //     }
    //     return true; 
    //     // This return value doesn't have much meaning as it contains a loop.
    //     // The return value is not significant because the loop has already completed its task.
    //     // To make the return value more meaningful, the function should return a Promise object 
    //     // to notify the caller when it is ready to start working.
    // }

    void Server::receiveMessages(int sock)
    // This function is mapped 1:1 to a socket.
    // Since the function is not written asynchronously and loops inside a thread,
    // a different structure is required to handle receiving data from multiple sockets.
    {
        char buffer[BUFFER_SIZE];
        socklen_t len = sizeof(server_addr);

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int n = recvfrom(sock, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&server_addr, &len);
            if (n < 0) {
                std::cerr << "Failed to receive message." << std::endl;
                break;
            }
            buffer[n] = '\0';
            std::cout << "Server: " << buffer << std::endl;
        }
    }

    void Server::sendMessages(int sock)
    {
        std::string message;
        socklen_t len = sizeof(server_addr);

        while (true) {
            std::getline(std::cin, message);
            sendto(sock, message.c_str(), message.length(), MSG_CONFIRM, (const struct sockaddr *)&server_addr, len);
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