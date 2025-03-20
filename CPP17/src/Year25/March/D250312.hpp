#include <iostream>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <net/if.h>

namespace Y25M3
{
    using namespace std;
    constexpr int BUFFER_SIZE = 65536;

    void sample()
    {
        std::cout << "Hello, March\n";
        return;
    }

    // 패킷 처리 함수
    void process_packet(unsigned char *bufer, int size)
    {
        // #include <linux/if_ether.h>

        /// @brief ethhdr 설명
        // struct ethhdr {
        // 	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
        // 	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
        // 	__be16		h_proto;		/* packet type ID field	*/
        // } __attribute__((packed));

        /// @brief arp header 설명
        // ar_hrd: 1 이면 이더넷
        // ar_pro: 0x0800 이면 IPv4
        // ar_hln: 하드웨어 주소 길이(이더넷인 경우 6)
        // ar_pln: 프로토콜 주소 길이(IPv4인 경우 4)
        // ar_op: 1 이면 요청, 2 이면 응답
        // struct arphdr {
        //     __be16		ar_hrd;		/* format of hardware address	*/
        //     __be16		ar_pro;		/* format of protocol address	*/
        //     unsigned char	ar_hln;		/* length of hardware address	*/
        //     unsigned char	ar_pln;		/* length of protocol address	*/
        //     __be16		ar_op;		/* ARP opcode (command)		*/

        // #if 0
        //      /*
        //       *	 Ethernet looks like this : This bit is variable sized however...
        //       */
        //     unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
        //     unsigned char		ar_sip[4];		/* sender IP address		*/
        //     unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
        //     unsigned char		ar_tip[4];		/* target IP address		*/
        // #endif
        // };
        struct ethhdr *eth = (struct ethhdr *)buffer;


        // struct iphdr
        // {
        //     #if __BYTE_ORDER == __LITTLE_ENDIAN unsigned int ihl : 4;
        //         /* header length */ unsigned int version : 4;
        //     /* version */ #elif __BYTE_ORDER == __BIG_ENDIAN unsigned int version : 4;
        //         /* version */ unsigned int ihl : 4;
        //     /* header length */ #else #error "Please fix <bits/endian.h>" #endif u_int8_t tos;
        //     /* type of service */  u_int16_t tot_len;
        //     /* total length */  u_int16_t id;
        //     /* identification */  u_int16_t frag_off;
        //     /* fragment offset field */  u_int8_t ttl;
        //     /* time to live */  u_int8_t protocol;
        //     /* protocol */  u_int16_t check;
        //     /* checksum */  u_int32_t saddr;
        //     /* source address */  u_int32_t daddr;
        //     /* dest address */  /*The options start here. */  
        // };
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));

        char src_ip[INET_ADDRSTRLEN], dest_ip[INET_ADDRSTRLEN];

        // <arpa/inet.h> -> IPv4, IPv6를 human-readable text로 변환
        inet_ntop(AF_INET, &ip->saddr, src_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &ip->daddr, dest_ip, INET_ADDRSTRLEN);

        std::cout << "Captured packet : " << src_ip << " -> " << dest_ip << "\n";

        // 트겅 목적지로 포워딩
        if (strcmp(dest_ip, "192.168.1.100") == 0)
        {
            forward_packet(socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP)), buffer, size, "eth1");
        }
    }

    // 패킷 포워딩 함수
    void forward_packet(int sockfd, unsigned char *buffer, int size, const char *out_iface)
    {
        struct ifreq if_idx;
        struct sockaddr_ll socket_address;

        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, out_iface, IFNAMSIZ-1);

        // ioctl : HW 제어와 상태 정보. 
        // 
        if(ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        {
            perror("Error getting interface index");
            return;
        }

        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        socket_address.sll_halen = ETH_ALEN;
        memcpy(socket_address_sll_addr, buffer + 6, ETH_ALEN);

        // 패킷 전송
        if (sendto(sockfd, buffer, size, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
            perror("Packet Forward Error");
        } else {
            std::cout << "Packet Forwarded to " << out_iface << std::endl;
        }
    }

    void Ex1()
    {
        int sockfd;
        unsigned char buffer[BUFFER_SIZE];

        // RAW_SOCKET (AF_PACKET: Ethernet Frame Capture)
        sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
        if(sockfd < 0)
        {
            perror("Socket Error");
            return 1;
        }

        std::cout << "Packet sniffer & forwarding started...\n";

        while(true)
        {
            struct sockaddr saddr;
            socklen_t saddr_len = sizeof(saddr);

            // 패킷 수신
            int packet_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, &saddr, &saddr_len);
            if(packet_size < 0)
            {
                perror("Packet Receive Error");
                continue;
            }

            // 패킷 처리 (목적지 확인 후 포워딩)
            process_packet(buffer, packet_size);
        }

        close(sockfd);
        return 0;
    }

}


