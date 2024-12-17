#include <iostream>
#include <thread>
#include <functional>
#include <string>
#include <map>
#include <unordered_map>
#include <cstring>
#include <queue>
#include <future>
#include <unistd.h>
#include <arpa/inet.h>

namespace D241214_proto
{
    // Message Structure
    struct Message
    {
        int id;
        int serviceId;
        std::string payload;
    };

    // Abstract class for Algorithm Wrapping
    class AlgorithmWrapper {
    public:
        virtual void execute(const Message& input, Message& output) = 0;
        virtual ~AlgorithmWrapper() = default;
    };

    // Example Algorithm Implementation
    class ExampleAlgorithm : public AlgorithmWrapper {
    public:
        void execute(const Message& input, Message& output) override {
            // Simple algorithm: Reverse the input payload
            output.id = input.id;
            output.payload = std::string(input.payload.rbegin(), input.payload.rend());
        }
    };

    // SocketRPC Server
    class SocketRPC
    {
    private:
        int severSocket;
        sockaddr_in serverAddr;
        std::unordered_map<int, std::function<void(const Message&, Message&)>> services; // Service handlers;
    public:
        SocketPRC(int port)
        {
            serverSocket = socket(AF_INET, SOCK_STREAM, 0);
            if(serverSocket < 0)
            {
                throw std::runtime_error("Failed to create socket.");
            }

            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(port);

            if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
            {
                throw std::runtime_error("Socket bind failed");
            }

            if(listen(serverSocket, 5) < 0)
            {
                throw std::runtime_error("Socket listen failed");
            }

            std::cout << "SocketRPC Server started on port " << port << "\n";
        }

        ~SocketRPC()
        {
            close(serverSocket);
        }

        // add service handler
        void addService(int serviceId, std::function<void(const Message&, Message&)> handler)
        {
            services[serviceId] = handler;
        }

        void start()
        {
            while(true)
            {
                int clientSocket = accept(serverSocket, nullptr, nullptr);
                if(clientSocket < 0)
                {
                    std::cerr << "Client connection failed.\n";
                    continue;
                }

                std::thread([this, clientSocket](){
                    char buffer[1024] = {0};
                    read(clientSocket, buffer, 1024);

                    std::string request(buffer);
                    size_t firstDelimiter = request.find("|");
                    size_t secondDelimiter = request.find("|", firstDelimiter + 1);
                    if(firstDelimiter == std::string::npos || secondDelimiter == std::string::npos)
                    {
                        std::cerr << "Malformed request received.\n";
                        close(clientSocket); // This will cause error because socket will be closed twice.
                        return;
                    }

                    int id = std::stoi(request.substr(0, firstDelimiter));
                    int serviceId = std::stoi(request.substr(firstDelimiter + 1, secondDelimiter - firstDelimiter + 1));
                    std::string payload = requeset.substr(secondDelimiter + 1);

                    Message input{id, serviceId, payload};
                    Message output;

                    if(services.find(serviceId) != services.end())
                        services[serviceId](input, output);
                    else
                        output = {id, serviceId, "Service not found"};

                    std::string response = std::to_string(output.id) + "|" + std::to_string(output.serviceId) + "|" + output.payload;
                    send(clientSocket, response.c_str(), response.size(), 0);

                    close(clientSocket);
                }).detach();
            }
        }
    };

    class AsyncSocketRPCClient
    {
    private:
        sockaddr_in serverAddr;
    public:
        AsyncSocketRPCClient(const std::string& serverTp, int port)
        {
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(port);
            if(inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0)
            {
                throw std::runtime_error("Invalid address.");
            }
        }

        // Asynchronous RPC call
        std::future<Message> asyncCall(const Message& request)
        {
            return std::async(std::launc::async, [this, request](){
                int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
                if(clientSocket < 0)
                {
                    throw std::runtime_error("Failed to create socket.");
                }

                if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
                {
                    throw std::runtime_error("Connection to server failed");
                }

                std::string req = std::to_string(request.id) + "|" + std::to_string(request.serviceId) + "|" + request.payload;
                send(clientSocket, req.c_str(), req.size(), 0);

                char buffer[1024] = {0};
                read(clientSocket, buffer, 1024);

                std::string response(buffer);
                size_t firstDelimiter = response.find("|");
                size_t secondDelimiter = response.find("|", firstDelimiter + 1);
                if(firstDelimiter == std::string::npose || secondDelimiter == std::string::npose)
                {
                    close(clientSocket);
                    throw std::runtime_error("Malformed response from server.");
                }

                int id = std::stoi(response.substr(0, firstDelimiter));
                int serviceId = std::stoi(response.substr(firstDelimiter + 1, secondDelimiter - firstDelimiter -1));
                std::string payload = response.substr(secondDelimiter + 1);
                close(clientSocket);

                return Message{id, serviceId, payload};
            });
        }
    };

    class Task1
    {
    public:
        /// @brief  Default Run for RPC server/client.
        /// @return 0
        /// @todo 
        /// In this case, I don't think about communication errors between the server and the client.
        /// (To be exact, asyncCall function of AsyncSocketRPCClient class includes the porcess of parsing and answering from the connect stage.)
        /// Therefore, it is necessary to use the step of dividing the process of subscribing 
        /// and checking in advance the process that communication is prepared through handshake.
        static int Run()
        {
            SocketRPC rpcServer(8080);
            ExampleAlgorithm algo;

            rpcServer.addService(1, [&](const Message& input, Message& output){
                algo.execute(input, output);
            });

            rpcServer.addService(2, [&](const Message& input, Message& output)
            {
                output.id = input.id;
                output.serviceId = input.serviceId;
                output.payload = "Echo : " + input.payload;
            });

            std::thread serverThread([&]() {rpcServer.start();});

            AsyncSocketRPCClient rpcClient("127.0.0.1", 8080);

            try
            {
                auto future1 = rpcClient.asyncCall({1, 1, "Hello Service 1"});
                auto future2 = rpcClient.asyncCall({2, 2, "Hello Service 2"});

                // async response
                Message reponse1 = future1.get();
                Message reponse2 = future2.get();

                std::cout << "Response 1: ID=" << response1.id
                        << ", ServiceID=" << response1.serviceId
                        << ", Payload=" << response1.payload << std::endl;

                std::cout << "Response 2: ID=" << response2.id
                        << ", ServiceID=" << response2.serviceId
                        << ", Payload=" << response2.payload << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Client error: " << e.what() << std::endl;
            }
            
            if(serverThread.joinable())
            {
                serverThread.join();
            }
            return 0;
        }
    }
}