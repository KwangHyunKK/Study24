#include <gtest/gtest.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <functional>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <regex>
#include <map>

namespace Demo
{
    namespace Email
    {
        struct Email
        {
            std::string from;
            std::string to;
            std::string subject;
            std::string body;
            std::vector<std::pair<std::string, std::string>> attachments; // <filename, content>
        }; // Email struct

        std::string guessMimeType(const std::string &filename)
        {
            static std::map<std::string, std::string> mimeMap = {
                {".txt", "text/plain"},
                {".html", "text/html"},
                {".jpg", "image/jpeg"},
                {".png", "image/png"},
                {".pdf", "application/pdf"},
                {".zip", "application/zip"},
            };
            auto pos = filename.find_last_of('.');
            if (pos != std::string::npos)
            {
                std::string ext = filename.substr(pos);
                auto it = mimeMap.find(ext);
                if (it != mimeMap.end())
                    return it->second;
            }
            return "application/octet-stream";
        }

        class SmtpClient
        {
        public:
            SmtpClient() : ctx(nullptr), ssl(nullptr), sockfd(-1)
            {
                SSL_load_error_strings();
                OpenSSL_add_ssl_algorithms();
                const SSL_METHOD *method = TLS_client_method();
                ctx = SSL_CTX_new(method);
            }

            ~SmtpClient()
            {
                disconnect();
                SSL_CTX_free(ctx);
                EVP_cleanup();
            }

            bool connect(const std::string &host, int port)
            {
                struct hostent *server = gethostbyname(host.c_str());
                if (!server)
                    return false;

                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0)
                    return false;

                sockaddr_in serv_addr{};
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(port);
                memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

                if (::connnect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                    return false;

                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, sockfd);
                if (SSL_connect(ssl) <= 0)
                    return false;

                return getResponse().starts_with("220");
            } // connect

            bool sendCommand(const std::string &cmd, const std::string &expectCode)
            {
                std::string fullCmd = cmd + "\r\n";
                SSL_write(ssl, fullCmd.c_str(), fullCmd.size());
                std::string response = getResponse();
                return response.starts_with(expectCode);
            }

            bool authenticate(const std::string &user, const std::string &pass)
            {
                sendCommand("EHLO client", "250");
                sendCommand("AUTH LOGIN", "334");
                sendCommand(base64Encode(user), "334");
                return sendCommand(base64Encode(pass), "235");
            }

            bool sendMail(const std::string &from, const std::string &to, const std::string &subject, const std::string &body)
            {
                if (!sendCommand("MAIL FROM:<" + from + ">", "250"))
                    return false;
                if (!sendCommand("RCPT TO:<" + to + ">", "250"))
                    return false;
                if (!sendCommand("DATA", "354"))
                    return false;

                std::ostringstream msg;
                msg << "Subject: " << subject << "\r\n";
                msg << "To: " << to << "\r\n";
                msg << "From: " << from << "\r\n";
                msg << "\r\n"
                    << body << "\r\n." << "\r\n";
                SSL_write(ssl, msg.str().c_str(), msg.str().length());
                return getResponse().starts_with("250");
            }

            void disconnect()
            {
                if (ssl)
                {
                    sendCommand("QUIT", "221");
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                    ssl = nullptr;
                }
                if (sockfd != -1)
                {
                    close(sockfd);
                    sockfd = -1;
                }
            }
            std::string generateMimeMessage(const Email &mail)
            {
                std::ostringstream msg;
                std::string boundary = "--boundary" + std::to_string(std::time(nullptr));

                msg << "Subject: " << mail.subject << "\r\n";
                msg << "To: " << mail.to << "\r\n";
                msg << "From: " << mail.from << "\r\n";
                msg << "MIME-Version: 1.0\r\n";
                msg << "Content-Type: multipart/mixed; boundary=\"" << boundary << "\"\r\n\r\n";

                msg << "--" << boundary << "\r\n";
                msg << "Content-Type: text/plain; charset=\"utf-8\"\r\n\r\n";
                msg << mail.body << "\r\n\r\n";

                for (const auto &[filename, _] : mail.attachments)
                {
                    std::ifstream ifs(filename, std::ios::binary);
                    std::ostringstream oss;
                    oss << ifs.rdbuf();
                    std::string encoded = base64Encode(oss.str());
                    std::string mimeType = guessMimeType(filename);

                    msg << "--" << boundary << "\r\n";
                    msg << "Content-Type: " << mimeType << "; name=\"" << filename << "\"\r\n";
                    msg << "Content-Transfer-Encoding: base64\r\n";
                    msg << "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n\r\n";
                    msg << encoded << "\r\n\r\n";
                }
                msg << "--" << boundary << "--\r\n." << "\r\n";
                return msg.str();
            }

        private:
            SSL_CTX *ctx;
            SSL *ssl;
            int sockfd;

            std::string getResponse()
            {
                char buffer[4096] = {0};
                int bytes = SSL_read(ssl, buffer, sizeof(buffer));
                return std::string(buffer, bytes);
            }

            std::string base64Encode(const std::string &in)
            {
                BIO *bio, *b64;
                BUF_MEM *bufferPtr;

                b64 = BIO_new(BIO_f_base64());
                bio = BIO_new(BIO_s_mem());
                bio = BIO_push(b64, bio);

                BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
                BIO_write(bio, in.c_str(), in.size());
                BIO_flush(bio);
                BIO_get_mem_ptr(bio, &bufferPtr);

                std::string encoded(bufferPtr->data, bufferPtr->length);
                BIO_free_all(bio);
                return encoded;
            }
        }; // SmtpClient class

        // // GTest
        // TEST(SmtpClientTest, SendNaverMail)
        // {
        //     SmtpClient client;
        //     ASSERT_TRUE(client.connect("smtp.naver.com", 465));
        //     ASSERT_TRUE(client.authenticate("your_naver_id", "your_app_password"));
        //     ASSERT_TRUE(client.sendMail("your_email@naver.com", "to_email@example.com",
        //                                 "Test Subject", "This is a test message from GTest."));
        //     client.disconnect();
        // }

        // // main
        // int main(int argc, char **argv)
        // {
        //     ::testing::InitGoogleTest(&argc, argv);
        //     return RUN_ALL_TESTS();
        // }

        class Pop3Client
        {
        public:
            Pop3Client() : ctx(nullptr), ssl(nullptr), sockfd(-1)
            {
                SSL_load_error_strings();
                OpenSSL_add_ssl_algorithms();
                ctx = SSL_CTX_new(TLS_client_method());
            }

            ~Pop3Client()
            {
                disconnect();
                SSL_CTX_free(ctx);
                EVP_cleanup();
            }

            bool connect(const std::string &host, int port)
            {
                struct hostent *server = gethostbyname(host.c_str());
                if (!server)
                    return false;

                sockfd = socket(AF_INET, SOCKSTREAM, 0);
                if (sockfd < 0)
                    return false;

                sockaddr_in serv_addr{};
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(port);
                memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

                if (::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                    return false;
                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, sockfd);
                if (SSL_connect(ssl) <= 0)
                    return false;

                return getResponse().starts_with("+OK");
            }

            bool login(const std::string &user, const std::string &pass)
            {
                return sendCommand("USER " + user, "+OK") && sendCommand("PASS " + pass, "+OK");
            }

            void setMailReceivedCallback(std::function<void(const std::string &)> cb)
            {
                onMailReceived = std::move(cb);
            }

            void fetchLatest()
            {
                sendCommand("STAT", "+OK");
                sendCommand("LIST", "+OK");
                sendCommand("RETR 1", "+OK");
                std::string mail = getMultiLineResponse();
                if (onMailReceived)
                    onMailReceived(mail);
            }

            void disconnect()
            {
                if (ssl)
                {
                    sendCommand("QUIT", "+OK");
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                    ssl = nullptr;
                }
                if (sockfd != -1)
                {
                    close(sockfd);
                    sockfd = -1;
                }
            }

        private:
            SSL_CTX *ctx;
            SSL *ssl;
            int sockfd;
            std::function<void(const std::string &)> onMailReceived;

            bool sendCommand(const std::string &cmd, const std::string &expect)
            {
                std::string full = cmd + "\r\n";
                SSL_write(ssl, full.c_str(), full.size());
                return getResponse().starts_with(expect);
            }

            std::string getResponse()
            {
                char buffer[4096] = {0};
                int bytes = SSL_read(ssl, buffer, sizeof(buffer));
                return std::string(buffer, bytes);
            }

            std::string getMultiLineResponse()
            {
                std::ostringstream oss;
                char buffer[4096];
                int bytes;
                while ((bytes = SSL_read(ssl, buffer, sizeof(buffer))) > 0)
                {
                    oss << std::string(buffer, bytes);
                    if (oss.str().find("\r\n.\r\n") != std::string::npos)
                        break;
                }
                return oss.str();
            }
        }; // Pop3Client class

        // TEST(SmtpClientTest, SendNaverMailWithAttachment) {
        //     SmtpClient client;
        //     ASSERT_TRUE(client.connect("smtp.naver.com", 465));
        //     ASSERT_TRUE(client.authenticate("your_naver_id", "your_app_password"));
        //     Email mail{ "your_email@naver.com", "to_email@example.com", "Test With File", "This is a test with attachment." };
        //     mail.attachments.push_back("/path/to/file.txt");
        //     ASSERT_TRUE(client.sendMail(mail));
        //     client.disconnect();
        // }
        
        // TEST(Pop3ClientTest, ReceiveMail) {
        //     Pop3Client pop;
        //     ASSERT_TRUE(pop.connect("pop.naver.com", 995));
        //     ASSERT_TRUE(pop.login("your_naver_id", "your_app_password"));
        //     pop.setMailReceivedCallback([](const std::string& mailBody) {
        //         std::cout << "\n[RECEIVED MAIL]\n" << mailBody << std::endl;
        //         ASSERT_FALSE(mailBody.empty());
        //     });
        //     pop.fetchLatest();
        //     pop.disconnect();
        // }
    } // namespace Email
} // namespace Demo