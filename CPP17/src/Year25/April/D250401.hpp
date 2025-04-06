// NetworkInfo
#include <iostream>
#include <vector>
#include <string>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Y25M4
{
    class NetworkInterfaceInfo
    {
    public:
        std::string name_;
        std::string ipAddress_;
        std::string netmask_;
        std::string macAddress_;
        bool isUp_;

        NetworkInterfaceInfo(const std::string &_name="",
                             const std::string &_ipAddress="",
                             const std::string &_netmask="",
                             const std::string &_macAddress="",
                             bool _isUp=false)
            : name_(_name), ipAddress_(_ipAddress), netmask_(_netmask), macAddress_(_macAddress), isUp_(_isUp) {}

        void printDetail() const
        {
            std::cout << std::setw(30) << "Interface: " << name_ << "\n";
            std::cout << std::setw(30) << "  IP Address : " << ipAddress_ << "\n";
            std::cout << std::setw(30) << "  Netmask    : " << netmask_ << "\n";
            std::cout << std::setw(30) << "  MAC Address: " << macAddress_ << "\n";
            std::cout << std::setw(30) << "  Status     : " << (isUp_ ? "UP" : "DOWN") << "\n";
        }

        bool operator==(const NetworkInterfaceInfo &otherNet_) const
        {
            return name_ == otherNet_.name_ &&
                   ipAddress_ == otherNet_.ipAddress_ &&
                   netmask_ == otherNet_.netmask_ &&
                   macAddress_ == otherNet_.macAddress_;
        }

        bool operator!=(const NetworkInterfaceInfo &otherNet_)
        {
            return !this->operator==(otherNet_);
        }
    };

    class NetWorkInterfaceManager
    {
    private:
        std::vector<NetworkInterfaceInfo> interfaces_;

        std::string getMACAddress(struct ifaddrs *ifa)
        {
            if (ifa->ifa_addr->sa_family == AF_PACKET)
            {
                struct sockaddr_ll *s = (struct sockaddr_ll *)ifa->ifa_addr;
                std::ostringstream mac;
                for (int i = 0; i < s->sll_halen; ++i)
                {
                    mac << std::hex << std::setfill('0') << std::setw(2) << (int)s->sll_addr[i];
                    if (i != s->sll_halen - 1)
                        mac << ":";
                }
                return mac.str();
            }
            return "";
        }

        NetWorkInterfaceManager() = default;

    public:
        // Use for singleton pattern
        static NetWorkInterfaceManager& GetInstance()
        {
            static NetWorkInterfaceManager manager;
            return manager;
        }

        void loadInterface()
        {
            interfaces_.clear();
            struct ifaddrs *ifaddr = nullptr;
            getifaddrs(&ifaddr); // in ifaddrs.h

            // TODO: Unique Ptr 또는 Shared Ptr로 Wrapping 가능?
            for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
            {
                if (ifa->ifa_addr == nullptr)
                    continue;

                NetworkInterfaceInfo nii;

                nii.name_ = ifa->ifa_name;
                nii.isUp_ = ifa->ifa_flags & IFF_UP;

                if (ifa->ifa_addr->sa_family == AF_INET)
                {
                    char ip[INET_ADDRSTRLEN], mask[INET_ADDRSTRLEN];
                    void *addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                    void *mask_addr = &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr;
                    inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);
                    inet_ntop(AF_INET, mask_addr, mask, INET_ADDRSTRLEN);
                    nii.ipAddress_ = ip;
                    nii.netmask_ = mask;
                }

                if (ifa->ifa_addr->sa_family == AF_PACKET)
                {
                    nii.macAddress_ = getMACAddress(ifa);
                }

                // 중복 추가 방지
                bool exists = false;
                if (interfaces_.end() != std::find(interfaces_.begin(), interfaces_.end(), nii))
                {
                    exists = true;
                }

                if (!exists)
                    interfaces_.emplace_back(std::move(nii));
            }

            freeifaddrs(ifaddr);
        }

        void printAllInterfaces() const
        {
            for (const auto &iface : interfaces_)
            {
                iface.printDetail();
                std::cout << "-----------------------\n";
            }
        }

        const NetworkInterfaceInfo *findByIP(const std::string &ip) const
        {
            for (const auto &iface : interfaces_)
            {
                if (iface.ipAddress_ == ip)
                    return &iface;
            }
            return nullptr;
        }

        const NetworkInterfaceInfo *findByMac(const std::string &mac) const
        {
            for (const auto &iface : interfaces_)
            {
                if (iface.macAddress_ == mac)
                    return &iface;
            }
            return nullptr;
        }

        void printNetmask(const std::string &ifaceName) const
        {
            for (const auto &iface : interfaces_)
            {
                if (iface.name_ == ifaceName)
                {
                    std::cout << "Netmask for " << ifaceName << ": " << iface.netmask_ << "\n";
                    return;
                }
            }
            std::cout << "Interface not found.\n";
        }

        void printEthernetInterfaces() const
        {
            for (const auto &iface : interfaces_)
            {
                if (iface.name_.find("eth") != std::string::npos ||
                    iface.name_.find("en") == 0)
                {
                    std::cout << "Ethernet Interface Info:\n";
                    iface.printDetail();
                    std::cout << "-----------------------\n";
                }
            }
        }
    };

    int Run()
    {
        auto manager = std::move(NetWorkInterfaceManager::GetInstance());
        manager.loadInterface();

        std::cout << "🔧 All Interfaces:\n";
        manager.printAllInterfaces();

        std::string targetIP = "172.27.147.175";
        const auto* ifaceByIP = manager.findByIP(targetIP);
        if (ifaceByIP) {
            std::cout << "🔍 Found interface by IP " << targetIP << ":\n";
            ifaceByIP->printDetail();
        } else {
            std::cout << "❌ Interface not found for IP " << targetIP << "\n";
        }

        std::string targetMAC = "00:15:5d:ed:d4:ce";
        const auto* ifaceByMAC = manager.findByMac(targetMAC);
        if (ifaceByMAC) {
            std::cout << "🔍 Found interface by MAC " << targetMAC << ":\n";
            ifaceByMAC->printDetail();
        } else {
            std::cout << "❌ Interface not found for MAC " << targetMAC << "\n";
        }

        std::string targetInterface = "eth0";
        manager.printNetmask(targetInterface);

        std::cout << "🔎 Ethernet Interfaces Detail:\n";
        manager.printEthernetInterfaces();

        return 0;
    }
}