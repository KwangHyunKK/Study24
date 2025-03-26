// Update D250312 : Router Code with process_packet & forwarding
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <queue>
#include <thread>
#include <mutex>
#include <sys/ioctl.h>
#include <condition_variable>

namespace Y25M3
{
    class Router
    {
    public:
        Router()=default;
        Router(const Router&)=delete;
        Router& operator=(const Router&)=delete;
        Router(Router&&)=default;
        Router& operator=(Router&&)=delete;
        virtual ~Router();
        static constexpr uint BUFFER_SIZE=65536;
        static constexpr uint MAX_THREADS=4;
        static constexpr uint MAX_RETRIES=3;

        void startThreadPool();

    private:

        void process_packet(unsigned char* _buffer, int _size);
        void forward_packet(int _sockfd, unsigned char* _buffer, int _size, const char* _out_iface);
        void packet_listener();
        std::queue<std::vector<unsigned char>> packetQueue_;
        std::mutex queueMutex_;
        std::vector<std::thread> workers_;
        std::condition_variable queueCV_;
        bool stopProcessing_;
    };

    void Router::startThreadPool()
    {
        workers_.emplace_back(std::thread(&Router::packet_listener, this));
        auto workerThread = [this]()
        {
            while(!stopProcessing_)
            {
                std::vector<unsigned char> packet;
                {
                    std::unique_lock<std::mutex> lock(queueMutex_);
                    queueCV_.wait(lock, [this]{return !packetQueue_.empty() || stopProcessing_; });
    
                    if(stopProcessing_)break;
    
                    packet = std::move(packetQueue_.front());
                    packetQueue_.pop();
                }
                process_packet(packet.data(), packet.size());
            }
        };
        for(int i=0;i<MAX_THREADS;++i)
        {
            workers_.emplace_back(workerThread);
        }
    }

    Router::~Router()
    {
        for(auto& worker:workers_)
        {
            if(worker.joinable())worker.join();
        }
    }

    void Router::packet_listener()
    {
        int sockfd;
        unsigned char buffer[BUFFER_SIZE];

        sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
        if(sockfd < 0)
        {
            perror("Socket Error");
            exit(1);
        }

        std::cout << "Packet Sniffer & Forwarding Started...\n";

        while(true)
        {
            struct sockaddr saddr;
            socklen_t saddr_len = sizeof(saddr);

            int packet_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, &saddr, &saddr_len);
            if(packet_size < 0)
            {
                perror("Packet Receive Error");
                continue;
            }

            {
                std::lock_guard<std::mutex> locK(queueMutex_);
                packetQueue_.emplace(buffer, buffer + packet_size);
            }
            queueCV_.notify_one();
        }
    }

    void Router::process_packet(unsigned char* buffer, int size)
    {
        // struct ethhdr { 이더넷 프레임의 MAC 주소 및 프로토콜 타입 정보 포함.
        //     unsigned char h_dest[ETH_ALEN];   // 목적지 MAC 주소 (6바이트)
        //     unsigned char h_source[ETH_ALEN]; // 출발지 MAC 주소 (6바이트)
        //     __be16 h_proto;                   // 상위 프로토콜 타입 (예: IPv4 - 0x0800)
        // };
        
        struct ethhdr* eth = (struct ethhdr*)buffer;

        // struct iphdr { IP 패킷의 버전, 길이, 출발지/목적지 주소 등을 포함.
        //     unsigned int ihl:4;        // 헤더 길이
        //     unsigned int version:4;    // IP 버전
        //     uint8_t tos;               // 서비스 타입
        //     uint16_t tot_len;          // 전체 길이
        //     uint16_t id;               // 식별자
        //     uint16_t frag_off;         // 단편 오프셋
        //     uint8_t ttl;               // TTL
        //     uint8_t protocol;          // 상위 프로토콜 (예: TCP - 6, UDP - 17)
        //     uint16_t check;            // 체크섬
        //     uint32_t saddr;            // 출발지 IP 주소
        //     uint32_t daddr;            // 목적지 IP 주소
        // };
        struct iphdr* ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));

        char src_ip[INET_ADDRSTRLEN], dest_ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &ip->saddr, src_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &ip->daddr, dest_ip, INET_ADDRSTRLEN);

        std::cout << "Captured Packet: " << src_ip << " -> " << dest_ip << "\n";

        // 특정 목적지로 포워딩
        if(strcmp(dest_ip, "192.168.1.100") == 0)
        {
            int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
            if(sockfd < 0)
            {
                perror("Socket Error");
                return;
            }
            forward_packet(sockfd, buffer, size, "eth1");
            close(sockfd);
        }
    }

    void Router::forward_packet(int sockfd, unsigned char* buffer, int size, const char* out_iface)
    {
        struct ifreq if_idx;
        // Raw Socket에서 사용하는 저수준 소켓 주소 구조체로, 이더넷 인터페이스와의 바인딩이나 전송에 사용.
        // struct sockaddr_ll {
        //     unsigned short sll_family;   // 항상 AF_PACKET
        //     unsigned short sll_protocol; // 프로토콜 (예: ETH_P_IP)
        //     int sll_ifindex;             // 인터페이스 인덱스
        //     unsigned short sll_hatype;  // 하드웨어 타입
        //     unsigned char sll_pkttype;  // 패킷 타입
        //     unsigned char sll_halen;    // 하드웨어 주소 길이
        //     unsigned char sll_addr[8];  // MAC 주소
        // };

        struct sockaddr_ll socket_address;

        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, out_iface, IFNAMSIZ-1);

        // 인터페이스 정보 ifreq
        // ioctl을 통해 interface index 등 얻음
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) 
        {
            perror("Error getting interface index");
            return;
        }

        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        socket_address.sll_halen = ETH_ALEN;
        memcpy(socket_address.sll_addr, buffer + 6, ETH_ALEN);

        int retries = 0;
        while(retries < MAX_RETRIES)
        {
            if (sendto(sockfd, buffer, size, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) 
            {
                perror("Packet Forward Error");
                retries++;
                std::cerr << "Retrying (" << retries << "/" << MAX_RETRIES << ")..." << std::endl;
                sleep(1);
            }
            else 
            {
                std::cout << "Packet Forwarded to " << out_iface << "\n";
                return;
            }
        }

            // 실패 시 로그 저장
            std::cerr << "Packet Forwarding Failed after " << MAX_RETRIES << " attempts." << std::endl;
            FILE* logFile = fopen("packet_failure.log", "a");
            if (logFile) 
            {
                fprintf(logFile, "Failed packet from %s\n", out_iface);
                fclose(logFile);
            }
    }

    void RouterSample()
    {
        Router router;
        router.startThreadPool();
        return;
    }
}