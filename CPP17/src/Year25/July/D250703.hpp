// dds_multithread_reliability.cpp
// Simplified DDS-lite with GUID, reliability (ACK/NACK), heartbeats, liveliness, and DomainParticipant maangement -> DDS2 namespace
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <chrono>
#include <random>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Y25M7
{
    namespace D250703
    {
        namespace DDS1
        {
            // struct
            // Topic type
            struct HelloMsg
            {
                std::string message;
                uint64_t timestamp;
            };

            // Topic class
            class Topic
            {
            public:
                std::string name;
                std::string type_name;
            };

            // Qos
            struct Qos
            {
                bool reliable = true;
                int max_latency_ms = 50;
            };

            // DomainParticipant
            {
            public:
                Publisher *create_publisher(const Qos &);
                Subscriber *create_subscriber(const Qos &);
            };

            // Publisher / Subscriber
            class Publisher
            {
            public:
                DataWriter *create_datawriter(const Topic &, const Qos &);
            };

            class Subscriber
            {
            public:
                DataReader *create_datareader(const Topic &, const Qos &);
            };

            // DataWriter / DataReader
            class DataWriter
            {
            public:
                void write(const HelloMsg &msg) // send with UDP
                {
                    std::ostringstream oss;
                    oss << msg.timestamp << "|" << msg.message;

                    std::string serialized = oss.str();
                    sendto(socket_fd, serialized.c_str(), serialized.size(), 0, ...);
                }
            };

            class DataReader
            {
            public:
                void set_listener(std::function<void(const HelloMsg &)>);

                void start_receive()
                {
                    std::thread([this]()
                                {
                        char buffer[1024];
                        while(true)
                        {
                            int len = recvfrom(socket_fd, buffer, sizeof(buffer), 0, ...);
                            if(len > 0)
                            {
                                std::istringstream isss(std::string(buffer, len));
                                HelloMsg msg;
                                std::getline(iss, msg.message, '|');
                                iss >> msg.timestamp;
                                if(listener)listener(msg);
                            }
                        } })
                        .detach();
                }
            }; // DataReader class

            void Run()
            {
                DomainParticipant participant;
                Topic topic{"Hello", "HelloMsg"};

                Publisher *pub = participant.create_publisher(Qos{true});
                DataWriter *writer = pub->create_datawriter(topic, Qos{true});

                Subscriber *sub = participant.create_subscriber(Qos{true});
                DataReader *reader = sub->create_datareader(topic, Qos{true});

                reader->set_listener([](const HelloMsg &msg)
                                     { std::cout << "[RECV] " << msg.message << " at " << msg.timestamp << "\n"; });

                HelloMsg msg{"Hello, DDS!", std::time(nullptr)};
                writer->write(msg);

                std::this_thread::sleep_for(std::chrono::seconds(1));
            } // Run function
        } // namespace DDS1
        namespace DDS2
        {

            // GUID and EntityID generation
            struct GUID_t
            {
                uint64_t prefix;   // participant-specific prefix (timestamp)
                uint32_t entityID; // endpoint-specific counter
            };

            class GUIDGenerator
            {
                static std::atomic<uint32_t> counter_;

            public:
                static GUID_t generate()
                {
                    uint32_t id = ++counter_;
                    uint64_t ts = std::chrono::steady_clock::now().time_since_epoch().count();
                    return GUID_t{ts, id};
                }
            };
            std::atomic<uint32_t> GUIDGenerator::counter_{0};

            // Serialize GUID to string
            std::string guid_to_string(const GUID_t &g)
            {
                return std::to_string(g.prefix) + "." + std::to_string(g.entityID);
            }

            // Thread - safe queue

            template <typename T>
            class ThreadSafeQueue
            {
                std::queue<T> queue_;
                std::mutex mtx_;
                std::condition_variable cv_;

            public:
                void push(const T &v)
                {
                    {
                        std::lock_guard<std::mutex> lock(mtx_);
                        queue_.push(v);
                    }
                    cv_.notify_one();
                }
                T wait_and_pop()
                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    cv_.wait(lock, [&]
                             { return !queue_.empty(); });
                    T v = queue_.front();
                    queue_.pop();
                    return v;
                }
            }; // ThreadSafeQueue class

            // Message + headers

            struct HelloMsg
            {
                std::string message;
                uint64_t timestamp;
            };

            enum class SubmessageKind : uint8_t
            {
                DATA = 1,
                ACKNACK = 2,
                HEARTBEAT = 3
            };

            struct Submessage
            {
                SubmessageKind kind;
                GUID_t writerGuid;
                uint32_t sequenceNumber;
                uint32_t ackNumber; // for ACKNCK
                HelloMsg payload;
            };

            // Endpoint base

            class Endpoint
            {
            public:
                GUID_t guid;
                Endpoint() : guid(GUIDGenerator::generate()) {}
            };

            // DomainParticipant : manage endpoints and deliver acks
            class DomainParticipant
            {
                std::unordered_map<std::string, WriterEndpoint *> writers_;
                std::unordered_map<std::string, ReaderEndpoint *> readers_;

            public:
                WriterEndpoint *create_writer(const std::string &ip, uint16_t port)
                {
                    auto w = new WriterEndpoint(this, ip, port);
                    writers_[guid_to_string(w->guid)] = w;
                    return w;
                }
                ReaderEndpoint *create_reader(uint16_t port)
                {
                    auto r = new ReaderEndpoint(this, port);
                    readers_[guid_to_string(r->guid)] = r;
                    return r;
                }

                void deliverAck(const std::string &writerGuid, uint32_t ackNum)
                {
                    auto it = writers_.find(writerGuid);
                    if (it != writers_.end())
                    {
                        it->second->onAck(ackNum);
                    }
                }
            }; // DomainParticipant

            // WriterEndpoint with reliability, heartbeat
            class WriterEndpoint : public Endpoint
            {
                ThreadSafeQueue<HelloMsg> writeQueue_;
                std::sthread sendThread_, heartbeatThread_;
                int sock_;
                struct sockaddr_in targetAddr_;
                std::atomic<uint32_t> seqNum_{0};
                std::mutex ackMtx_;
                std::unordered_map<uint32_t, HelloMsg> unacked_; // seq->msg
                DomainParticipant *participant_;
                bool running_{true};

            public:
                WriterEndpoint(DomainParticipant *dp, const std::string &ip, uint16_t port) : participant_(dp)
                {
                    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
                    memset(&targetAddr_, 0, sizeof(targetAddr_));
                    targetAddr_.sin_family = AF_INET;
                    targetAddr_.sin_port = htons(port);
                    inet_pton(AF_INET, ip.c_str(), &targetAddr_.sin_addr);

                    sendThread_ = std::thread(&WriteEndpoint::sendLoop, this);
                    heartbeatThread_ = std::thread(&WriterEndpoint::heartbeatLoop, this);
                }

                void write(const HelloMsg &msg)
                {
                    writeQueue_.push(msg);
                }

                void sendLoop()
                {
                    while (running_)
                    {
                        HelloMsg msg = writeQueue_.wait_and_pop();
                        uint32_t seq = ++seqNum_;
                        {
                            std::lock_guard<std::mutex> lock(actMtx_);
                            unacked_[seq] = msg;
                        }
                        Submessage sm{SubmessgeKing::DATA, guid, seq, 0, msg};
                        sendSubmessage(sm);
                    }
                }

                void heartbeatLoop()
                {
                    using namespace std::chrono_literals;
                    while (running_)
                    {
                        std::this_thread::sleep_for(1000ms);
                        uint32_t last = seqNum_.load();
                        Submessage hb{SubmessageKind::HEARTBEAT, guid, last, 0, {}};
                        sendSubmessage(hb);
                    }
                }

                void onAck(uint32_t ackNum)
                {
                    std::lock_guard<std::mutex> lock(ackMtx_);
                    // remove all <= ackNum
                    for (auto it = unacked_.begin(); it != unacked_.end();)
                    {
                        if (it->first <= ackNum)
                            it = unacked_.erase(it);
                        else
                            ++it;
                    }
                }

            private:
                void sendSubmessage(const Submessage &sm)
                {
                    // naive serialization
                    std::ostringstream oss;
                    oss << (uint8_t)sm.kind << "|"
                        << guid_to_string(sm.writerGuid) << "|"
                        << sm.sequenceNumber << "|"
                        << sm.ackNumber << "|"
                        << sm.payload.timestamp << "|"
                        << sm.payload.message;
                    std::string data = oss.str();
                    sendto(sock_, data.c_str(), data.size(), 0,
                           (struct sockaddr *)&targetAddr_, sizeof(targetAddr_));
                }
            }; // WriterEndpoint

            class ReaderEndpoint : public Endpoint
            {
                std::function<void(const HelloMsg &)> cb_;
                std::thread recvThread_;
                int sock_;
                uint16_t port_;
                DomainParticipant *participant_;
                bool running_{true};
                std::unordered_map<std::string, uint32_t> lastSeq_; // writerGuid->last seen seq
                std::unordered_map<std::string, std::chrono::steady_clock::time_point> lastHb_;

            public:
                ReaderEndPoint(DomainParticipant *dp, uint16_t port)
                    : participant_(dp), port_(port)
                {
                    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
                    sockaddr_in addr{};
                    addr.sin_family = AF_INET;
                    addr.sin_port = htons(port_);
                    addr.sin_addr.s_addr = INADDR_ANY;
                    bind(sock_, (sockaddr *)&addr, sizeof(addr));

                    recvThread_ = std::thread(&ReaderEndpoint::recvLoop, this);
                }

                void setListener(std::function<void(const HelloMsg &)> cb)
                {
                    cb_ = std::move(cb);
                }

            private:
                void recvLoop()
                {
                    char buf[1024];
                    while (running_)
                    {
                        int len = recvfrom(sock_, buf, sizeof(buf), 0, nullptr, nullptr) if (len <= 0) continue;
                        std::istringstream iss(std::string(buf, len));
                        int kind;
                        iss >> kind;
                        iss.ignore(1, '|');
                        std::string guidStr;
                        std::getline(iss, guidStr, '|');
                        uint32_t seq;
                        iss >> seq;
                        iss.ignore(1, '|');
                        uint32_t ack;
                        iss >> ack;
                        iss.ignore(1, '|');
                        uint64_t ts;
                        iss >> ts;
                        iss.ignore(1, '|');

                        std::string msg;
                        std::getline(iss, msg);

                        auto now = std::chrono::steady_clock::now();
                        lastHb_[guidStr] = now;

                        if (kind == (int)SubmessageKind::DATA)
                        {
                            HelloMsg hm{msg, ts};
                            if (cb_)
                                cb_(hm);
                            // send acknack
                            sendAck(guidStr, seq);
                            lastSeq_[guidStr] = seq;
                        }
                        else if (kind == (int)SubmessageKind::HEARTBEAT)
                        {
                            // update liveliness
                        }
                        else if (kind == (int)SubmessageKind::ACKNACK)
                        {
                            // find writer by GUID and call onAck
                            participant_->deliverAck(guidStr, ack);
                        }
                    }
                }

                void sendAck(const std::string &writerGuid, uint32_t seq)
                {
                    Submessage sm{SubmessageKind::ACKNACK, /*unused*/ {}, 0, seq, {}};
                    std::ostringstream oss;
                    oss << (uint8_t)sm.kind << "|"
                        << writerGuid << "|"
                        << sm.sequenceNumber << "|"
                        << sm.ackNumber;
                    std::string data = oss.str();
                    // reuse sock_ and target from discovery
                    // here naive send to fixed port
                    sendto(sock_, data.c_str(), data.size(), 0, (sockaddr *)& /* ... */, sizeof(/*...*/));
                }
            };

            int Run()
            {
                DomainParticipant dp;
                auto reader = dp.create_reader(6000);
                reader->setListener([](const HelloMsg &m)
                                    { std::cout << "[RX] " << m.message << " @ " << m.timestamp << "\n"; });
                auto writer = dp.create_writer("127.0.0.1", 6000);

                HelloMsg m{"Reliable Hello!", std::time(nullptr)};
                writer->write(m);

                std::this_thread::sleep_for(std::chrono::seconds(5));
                return 0;
            }
        } // namespace DDS2
    } // namespace D250703
} // namespace Y25M7