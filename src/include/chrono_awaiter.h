#include <chrono>
#include <thread>
#include <type_traits>

template<class Clock = std::chrono::high_resolution_clock>
    struct chrono_awaiter
    {
        using time_point = typename Clock::time_point;
        using duration = typename Clock::duration;
        time_point end;

        explicit chrono_awaiter(const time_point& end) noexcept : end{end} {}
        explicit chrono_awaiter(const duration& d) noexcept : end{Clock::now() + d} {}

        template<typename Rep, typename Period>
        explicit chrono_awaiter(std::chrono::duration<Rep, Period> d) noexcept
            : end{Clock::now() + std::chrono::duration_cast<duration>(d)} {}

        bool await_ready() const noexcept
        {
            return Clock::now() >= end;
        }
        void await_suspend(std::coroutine_handle<> cont) const
        {
            std::jthread ([cont, &end=end]() mutable 
            {
                std::this_thread::sleep_until(end);
                cont.resume();
            }).detach();
        }
        void await_resume() const noexcept {}
    };