#pragma once

#include <thread>
#include <type_traits>

#include "thread_pool.h"

template <class Task>
struct lambda_awaiter 
{
    Task action;
    using T = decltype(action());
    std::conditional_t<std::is_same_v<T, void>, void*, T> result{};
    std::exception_ptr ex;

    explicit lambda_awaiter(Task&& task) noexcept : action{std::move(task)} {}

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> cont) 
    {
        std::jthread([this, cont]() mutable 
        {
            try 
            {
                if constexpr(std::is_same_v<T, void>)
                {
                    action();
                }
                else
                {
                    result = action();
                }
            } 
            catch(...) 
            {
                ex = std::current_exception();
            }
            cont.resume();
        }).detach();
    }

    auto await_resume()
    {
        if (ex) 
        {
            std::rethrow_exception(ex);
        }
        if constexpr(!std::is_same_v<T, void>)
        {
            return std::move(result);
        }
    }
};

template <typename Task>
requires std::is_invocable_v<Task>
lambda_awaiter<Task> operator co_await(Task&& task) 
{
    return lambda_awaiter<Task>{std::forward<Task>(task)};
}
