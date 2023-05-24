#pragma once

#include <future>
#include <coroutine>
#include <type_traits>

template <typename T, typename... Args>
    requires (!std::is_void_v<T> && !std::is_reference_v<T>)
struct std::coroutine_traits<std::future<T>, Args...>
{
    struct promise_type
    {
        std::promise<T> promise;
        std::exception_ptr exception;

        std::future<T> get_return_object() noexcept
        {
            if (exception) 
            {
                std::rethrow_exception(exception);
            }
            return promise.get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_value(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        {
            promise.set_value(value);
        }
        void return_value(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        {
            promise.set_value(std::move(value));
        }
        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
            // promise.set_exception(std::current_exception());
        }
    };
};

template <typename... Args>
struct std::coroutine_traits<std::future<void>, Args...>
{
    struct promise_type
    {
        std::promise<void> promise;
        std::exception_ptr exception;

        std::future<void> get_return_object() noexcept
        {
            if (exception) 
            {
                std::rethrow_exception(exception);
            }
            return promise.get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_void() noexcept
        {
            promise.set_value();
        }
        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
            // promise.set_exception(std::current_exception());
        }
    };
};