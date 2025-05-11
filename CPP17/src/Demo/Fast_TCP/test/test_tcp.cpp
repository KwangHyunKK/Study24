#include "../src/tcpclient.cpp"
#include "../src/tcpserver.cpp"
#include <gtest/gtest.h>

namespace Demo
{
    namespace Fast_Tcp
    {
        TEST(TCPParallelTest, HighSpeedTransfer)
        {
            TCpServer server(SERVER_PORT);
            server.start();

            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::vector<std::thread> client_threads;

            for (int i = 0; i < TEST_CLIENT_COUNT; ++i)
            {
                client_threads.emplace_back([](){
                    TCPClient client("127.0.0.1", SERVER_PORT);
                    ASSERT_TRUE(client.connect_server());
                    client.send_loop(TEST_DURATION_SECONDS); 
                });
            }

            for (auto &th : client_threads)
            {
                if (th.joinable())
                    th.join();
            }
            server.stop();
        }
    }
}