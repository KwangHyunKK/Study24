#include <gtkmm.h>
#include <gdkmm/screen.h>
#include <sstream>
#include <iomanip>

#include <boost/asio.hpp>
#include <iostream>
#include <memory>

#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <tuple>
#include <algorithm>

namespace Y25M7
{
    namespace D250707
    {
        namespace GtkExample
        {
            class HexCalculator : public Gtk::Window
            {
            public:
                HexCalculator()
                {
                    set_title("Hex Calculator");
                    set_default_size(300, 400);
                    set_border_width(10) :

                                           auto css = Gtk::CssProvider::create();
                    css->load_from_path("style.css");
                    auto screen = Gdk::Screen::get_default();
                    Gtk::SytleContext::add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

                    // Vertical Box layout
                    m_VBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
                    m_VBox.set_spacing(6);
                    add(m_VBox);

                    // Entry
                    m_Entry.set_text("0");
                    m_Entry.set_alignment(1); // right alignment
                    m_Entry.set_margin_bottom(6);
                    m_vBox.pack_start(m_Entry, Gtk::PACK_SHRINK);

                    // Grid (Button)
                    m_Grid.set_row_spacing(5);
                    m_Grid.set_column_spacing(5);
                    m_Grid.get_style_context()->add_class("my-grid");
                    m_VBox.pack_start(m_Grid, Gtk::PACK_EXPAND_WIDGET);

                    // Button label and location
                    struct BtnInfo
                    {
                        std::string label;
                        int col, row, colspan;
                    };
                    const std::vector<BtnInfo> buttons = {
                        {"7", 0, 0, 1}, {"8", 1, 0, 1}, {"9", 2, 0, 1}, {"A", 3, 0, 1}, {"4", 0, 1, 1}, {"5", 1, 1, 1}, {"6", 2, 1, 1}, {"B", 3, 1, 1}, {"1", 0, 2, 1}, {"2", 1, 2, 1}, {"3", 2, 2, 1}, {"C", 3, 2, 1}, {"0", 0, 3, 1}, {"D", 1, 3, 1}, {"E", 2, 3, 1}, {"F", 3, 3, 1}, {"+", 0, 4, 1}, {"-", 1, 4, 1}, {"*", 2, 4, 1}, {"/", 3, 4, 1}, {"C", 0, 5, 1}, {"=", 1, 5, 3}};

                    for (auto &info : buttons)
                    {
                        auto btn = Gtk::make_managed<Gtk::Button>(info.label);
                        btn->set_margin(4);
                        btn->signal_clicked().connect(sigc::bind<Gtk::Button *>(
                            sigc::mem_func(*this, &HexCalculator::on_button_clicked), btn));

                        m_Grid.attach(*btn, info.col, info.row, info.colspan, 1);
                    }

                    show_all_children();
                }

            private:
                void on_button_clicked(Gtk::Button *btn)
                {
                    std::string label = btn->get_label();
                    std::string expr = m_Entry.get_text();

                    if (label == "C")
                    {
                        m_Entry.set_text("0");
                        return;
                    }
                    if (label == "=")
                    {
                        try
                        {
                            for (auto op : {'+', '-', '*', '/'})
                            {
                                auto pos = expr.find(op);
                                if (pos != std::string::npos)
                                {
                                    std::string a = expr.substr(0, pos);
                                    std::string b = expr.substr(pos + 10);
                                    int x = std::stoi(a, nullptr, 16);
                                    int y = std::stoi(b, nullptr, 16);
                                    int result = 0;
                                    switch (op)
                                    {
                                    case '+':
                                        result = x + y;
                                        break;
                                    case '-':
                                        result = x - y;
                                        break;
                                    case '*':
                                        result = x * y;
                                        break;
                                    case '/':
                                        result = y != 0 ? x / y : 0;
                                        break;
                                    }
                                    std::stringstream ss;
                                    ss << std::uppercase << std::hex << result;
                                    m_Entry.set_text(ss.str());
                                    break;
                                }
                            }
                        }
                        catch (...)
                        {
                            m_Entry.set_text("Error");
                        }
                        return;
                    }

                    if (expr == "0")
                        expr.clear();
                    m_Entry.set_text(expr + label);
                }

                Gtk::Box m_VBox{Gtk::ORIENTATION_VERTICAL};
                Gtk::Entry m_Entry;
                Gtk::Grid m_Grid;
            };

            int Run(int argc, char *argv[])
            {
                auto app = Gtk::Application::create(argc, argv, "com.example.hexcalc");
                HexCalculator window;
                return app->run(window);
            }
        } // GtkExample namespace

        namespace AsyncIO
        {
            using boost::asio::ip::tcp;

            // Async Session Class
            class Session : public std::enable_shared_from_this<Session>
            {
            public:
                explicit Session(tcp::socket socket)
                    : socket_(std::move(socket))
                {
                }
                void start() { do_read(); }

            private:
                void do_read()
                {
                    auto self = shared_from_this();
                    socket_async_read_some(
                        boost::asio::buffer(data_),
                        [this, self](boost::system::error_code ec, std::size_t length)
                        {
                            if (!ec)
                            {
                                do_write(length);
                            }
                        });
                }

                void do_write(std::size_t length)
                {
                    auto self = shared_from_this();
                    boost::asio::async_write(
                        socket_, boost::asio::buffer(data_, length),
                        [this, self](boost::system::error_code ec, std::size_t /*length*/)
                        {
                            if (!ec)
                            {
                                do_read();
                            }
                        });
                }

                tcp::socket socket_;
                enum
                {
                    max_length = 1024;
                }
                char data_[max_length];
            };

            class Server
            {
            public:
                Server(boost::asio::io_context &io_context, unsigned short port)
                    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
                {
                    do_accept();
                }

            private:
                void do_accept()
                {
                    acceptor_.async_accept(
                        [this](boost::system_error_code ec, tcp::socket socket)
                        {
                            if (!ec)
                            {
                                std::make_shared<Session>(std::move(socket))->start();
                            }
                            do_accept();
                        });
                }
                tcp::acceptor acceptor_;
            };

            void Run(int argc, char *argv[])
            {
                try
                {
                    if (argc != 2)
                    {
                        std::cerr << "Usage : " << argv[0] << " <port>\n";
                        return 1;
                    }

                    boost::asio::io_context io_context;
                    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
                    Server server(io_context, port);
                    io_context.run();
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                }
                return;
            }
        } // AsyncIO

        namespace EnhancedLZ77
        {
            // (offset, length, next char)
            using Token = std::tuple<int, int, char>;

            class EnhancedLZ77
            {
            public:
                EnhancedLZ77(int windowSize = 4096, int lookaheadSize = 18)
                    : W(windowSize), L(lookaheadSize) {}

                // Encoding : Input string -> token vector
                std::vector<Token> encode(const std::string &input)
                {
                    std::vector<Token> tokens;
                    int n = input.size();
                    int pos = 0;

                    // hash table
                    std::unorderd_map<char, std::deque<int>> occ;

                    while (pos < n)
                    {
                        int bestOffset = 0, bestLen = 0;

                        // candidate location
                        auto &dq = occ[input[pos]];
                        for (int prevPos : dq)
                        {
                            int maxMatch = std::min({L, n - pos, pos - prevPos});
                            int len = 0;
                            while (len < maxMatch && input[prevPos + len] == input[pos + len])
                                ++len;

                            if (len > bestLen)
                            {
                                bestLen = len;
                                bestOffset = pos - prevPos;
                            }
                        }

                        // lazy matching
                        if (bestLen > 1 && pos + 1 < n)
                        {
                            int nextOffset = 0, nextLen = 0;
                            auto &dq2 = occ[input[pos + 1]];
                            for (int prevPos : dq2)
                            {
                                int maxMatch = std::min({L, n - pos - 1, pos + 1 - prevPos});
                                int len = 0;
                                while (len < maxMatch && input[prevPos + len] == input[post + 1 + len])
                                {
                                    ++len;
                                }
                                if (len > nextLen)
                                {
                                    nextLen = len;
                                    nextOffset = (pos + 1) - prevPos;
                                }
                            }

                            if (nextLen > bestLen)
                            {
                                tokens.empalce_back(0, 0, input[pos]);
                                registerChar(input[pos], pos);
                                ++pos;
                                continue;
                            }
                        }

                        if (bestLen >= 2)
                        {
                            char nextChar = (pos + bestLen < n ? input[pos + bestLen] : '\0');
                            tokens.emplace_back(bestOffset, bestLen, nextchar);

                            for (int i = 0; i <= bestLen; ++i)
                                registerChar(input[pos + i], pos + i);

                            pos += bestLen + 1;
                        }
                        else
                        {
                            matching fail->literal
                                tokens.emplace_back(0, 0, input[pos]);
                            registerChar(input[pos], pos);
                            ++pos;
                        }
                    }
                    return tokens;
                }

                // decode
                std::string decode(const std::vector<Token> &tokens)
                {
                    std::string output;
                    for (auto [offset, length, c] : tokens)
                    {
                        if (length > 0)
                        {
                            int start = output.size() - offset;
                            for (int i = 0; i < length; ++i)
                            {
                                output.push_back(output[start + i]);
                            }
                            if (c != '\0')
                                output.push_back(c);
                        }
                    }
                    return output;
                }

            private:
                int W, L;

                void registerChar(char c, int pos)
                {
                    auto &dq = occ[c];
                    dq.push_back(pos);
                    if ((int)dq.size() > W / 2)
                        dq.pop_front();
                }

                std::unordered_map<char, std::deque<int>> occ;
            };

            int Run()
            {
                std::string text = "ABABABCABABABCABABABC";
                EnhancedLZ77 lz;
                auto tokens = lz.encode(text);

                std::cout << "Encoded tokens:\n";
                for (auto [off, len, c] : tokens)
                {
                    if (len > 0)
                        std::cout << "(" << off << "," << len << "," << c << ") ";
                    else
                        std::cout << "(0,0," << c << ") ";
                }
                std::cout << "\n";

                std::string restored = lz.decode(tokens);
                std::cout << "Decoded text: " << restored << "\n";
                return 0;
            }
        }
    }
} // Y25M7 namespace