
#include <coroutine>
#include <iostream>
#include <thread>
#include <memory>
#include <optional>

// practices_coroutines.cpp
// Demonstrates: (1) simple generator using coroutines, (2) basic async/await
// pattern where an awaitable runs work on a background thread and resumes
// the awaiting coroutine when complete.

// -----------------------------
// 1) Simple generator<int>
// -----------------------------
struct IntGenerator {
    struct promise_type {
        int current_value; // storage for yielded value
        // Return object: construct IntGenerator from coroutine handle
        auto get_return_object() {
            return IntGenerator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        // Suspend immediately after creation until 'begin' resumes
        std::suspend_always initial_suspend() { return {}; }
        // Suspend at the end; allow consumer to destroy
        std::suspend_always final_suspend() noexcept { return {}; }
        // Store yielded value and suspend
        std::suspend_always yield_value(int value) { current_value = value; return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h; // handle to the coroutine
    IntGenerator(std::coroutine_handle<promise_type> h) : h(h) {}
    IntGenerator(const IntGenerator&) = delete;
    IntGenerator(IntGenerator&& o) : h(o.h) { o.h = nullptr; }
    ~IntGenerator() { if (h) h.destroy(); }

    // iterator compatible interface for range-based for
    struct iterator {
        std::coroutine_handle<promise_type> h;
        bool operator!=(std::default_sentinel_t) const { return !h.done(); }
        void operator++() { h.resume(); }
        int operator*() const { return h.promise().current_value; }
    };

    iterator begin() {
        h.resume(); // advance to first yield
        return iterator{h};
    }
    std::default_sentinel_t end() { return {}; }
};

IntGenerator counter(int n) {
    for (int i = 0; i < n; ++i) {
        co_yield i; // yield 'i' back to the caller
    }
}

// -----------------------------
// 2) Async/await demo using an awaitable that runs work on a thread.
// -----------------------------

// ThreadAwaitable runs the provided function on a separate thread and
// resumes the awaiting coroutine when the function returns. The result is
// stored and returned by 'await_resume'. This is a lightweight demo of
// integrating coroutines with thread-based asynchronous work.
template<typename F>
struct ThreadAwaitable {
    using result_t = std::invoke_result_t<F>;
    F func; // work to run on background thread
    std::shared_ptr<std::optional<result_t>> result; // storage for result

    ThreadAwaitable(F f) : func(std::move(f)), result(std::make_shared<std::optional<result_t>>()) {}

    bool await_ready() const noexcept { return false; } // always suspend

    // When suspended, spawn a thread to run the work and resume the coroutine
    void await_suspend(std::coroutine_handle<> h) {
        auto res = result; // copy shared_ptr into thread
        std::thread([res, f = std::move(func), h]() mutable {
            try {
                *res = f(); // run the function and store the result
            } catch(...) {
                // store nothing on exception (demo only)
            }
            h.resume(); // resume the awaiting coroutine
        }).detach();
    }

    result_t await_resume() { return **result; } // return stored result
};

// A simple coroutine task that demonstrates awaiting a ThreadAwaitable
struct SimpleTask {
    struct promise_type {
        auto get_return_object() { return SimpleTask{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; } // start immediately
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;
    SimpleTask(std::coroutine_handle<promise_type> h) : h(h) {}
    ~SimpleTask(){ if (h) h.destroy(); }
};

// An example async function that awaits background computation
SimpleTask async_add_demo(int a, int b) {
    // awaitable runs a lambda on a background thread that computes a+b
    int sum = co_await ThreadAwaitable([a,b]{ return a + b; });
    // After await resumes, continue execution in this coroutine
    std::cout << "async_add_demo: " << a << " + " << b << " = " << sum << '\n';
}

void run_coroutines_practice() {
    std::cout << "--- Coroutines Practice ---\n";

    // 1) generator usage
    for (int v : counter(5)) {
        std::cout << v << ' ';
    }
    std::cout << '\n';

    // 2) simple async/await demo: start coroutine that schedules work on a thread
    // Keep the returned SimpleTask alive so the coroutine handle isn't destroyed
    // while the background thread is still running and plans to resume it.
    auto task = async_add_demo(10, 32); // starts immediately and prints when done

    // Wait a short time to allow background thread to finish in this demo.
    // In real programs use proper synchronization or an event loop instead.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

