#include <thread>
#include <type_traits>

template <class Task>
struct lambda_awaiter 
{
    Task action;
    using T = decltype(action());
    T result{};

    explicit lambda_awaiter(Task&& task) noexcept : action{std::move(task)} {}

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> cont) noexcept 
    {
        std::jthread([this, cont]() mutable 
        {
            result = action();
            cont();
        }).detach();
    }
    T await_resume() noexcept
    {
        return std::move(result);
    }
};

template <typename Task>
requires std::is_invocable_v<Task> lambda_awaiter<Task> operator co_await(Task&& task) noexcept 
{
    return lambda_awaiter<Task>{std::forward<Task>(task)};
}
