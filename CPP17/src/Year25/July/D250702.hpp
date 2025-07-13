#include <gtk/gtk.h>
#include <atomic>
#include <memory>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <thread>
#include <iostream>

namespace Y25M7
{
    namespace D250702
    {
        namespace gtk
        {
            static void
            print_hello(GtkWidget *widget, gpointer data)
            {
                g_print("Hello World\n");
            }

            static void
            activate(GtkApplication *app, gpointer user_data)
            {
                GtkWidget *window;
                GtkWidget *button;
                GtkWidget *box;

                window = gtk_application_window_new(app);
                gtk_window_set_title(GTK_WINDOW(window), "Window");
                gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

                box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
                gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
                gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

                gtk_window_set_child(GTK_WINDOW(window), box);

                button = gtk_button_new_with_label("Hello World");

                g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
                g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_destroy), window);

                gtk_box_append(GTX_BOX(box), button);

                gtk_window_present(GTK_WINDOW(window));
            }

            void Run(int argc, char **argv)
            {
                GtkApplication *app;
                int status;

                app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
                g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
                status = g_application_run(G_APPLICATION(app), argc, argv);
                g_object_unref(app);

                return status;
            }
        }
        namespace futex // C syscall
        {
            // Futex wrapper
            class Futex
            {
                std::atomic<int> v_{0};

            public:
                void wait(int expected)
                {
                    int c = v_.load(std::memory_order_acquire);
                    if (c != expected)
                        return; // if status has been changed, return.
                    syscall(SYS_futex, reinterpret_cast<int *>(&v_), FUTEX_WAIT, expected, nullptr, nullptr, 0);
                }

                // wake up
                void wake_one(int newVal)
                {
                    v_.store(newVal, std::memory_order_release);
                    syscall(SYS_futex, reinterpret_cast<int *>(&V_), FUTEX_WAKE, 1, nullptr, nullptr, 0);
                }
            };

            // Shared State
            template <typename T>
            struct SharedState
            {
                Futex futex;
                std::atomic<bool> ready{false};
                T value;
                std::exception_ptr exc;
            };

            // Future
            template <typename T>
            class MyFuture
            {
                std::shared_ptr<SharedState<T>> state_;

            public:
                explicit MyFuture(std::shared_ptr<SharedState<T>> s) : state_(std::move(s)) {}

                // blocking while value is setting.
                T get()
                {
                    while (!state_->ready.load(std::memory_order_acquire))
                    {
                        state_->futex.wait(0);
                    }

                    if (state_->exec)
                        std::rethrow_exception(state_->exc);
                    return std::move(state_->value);
                }
            };

            // Promise
            template <typename T>
            class MyPromise
            {
                std::shared_ptr<SharedState<T>> state_;

            public:
                MyPromise() : state_(std::make_shared<SharedState<T>>()) {}
                MyFuture<T> get_future() { return MyFuture<T>(state_); }

                void set_value(T v)
                {
                    state_->value = std::move(v);
                    state_->ready.store(true, std::memory_order_release);
                    state_->futex.wake_one(1);
                }

                void set_exception(std::exception_ptr e)
                {
                    state_->exc = e;
                    state_->ready.store(true, std::memory_order_release);
                    state_->futex.wake_one(1);
                }
            };

            int Run()
            {
                MyPromise<int> p;
                MyFuture<int> f = p.get_future();

                std::thread producer([&]()
                                     {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                try {
                    // throw std::runtime_error{"failure"};
                    p.set_value(42);
                }
                catch(...) {
                    p.set_exception(std::current_exception());
                } });

                std::thread consumer([&]()
                                     {
                try {
                    int result = f.get();
                    std::cout << "Got value: " << result << "\n";
                }
                catch (const std::exception& ex) {
                    std::cout << "Got exception: " << ex.what() << "\n";
                } });

                producer.join();
                consumer.join();
                return 0;
            }
        }
    }
}