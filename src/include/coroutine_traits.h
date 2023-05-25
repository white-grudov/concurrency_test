#pragma once

#include <future>
#include <coroutine>
#include <type_traits>

template <typename T, typename... Args>
    requires (!std::is_void_v<T> && !std::is_reference_v<T>)
struct std::coroutine_traits<std::future<T>, Args...>
{
    struct promise_type : std::promise<T>
    {
        std::future<T> get_return_object() noexcept
        {
            return this->get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_value(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        {
            this->set_value(value);
        }
        void return_value(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        {
            this->set_value(std::move(value));
        }
        void unhandled_exception() noexcept
        {
            this->set_exception(std::current_exception());
        }
    };
};

template <typename... Args>
struct std::coroutine_traits<std::future<void>, Args...>
{
    struct promise_type : std::promise<void>
    {
        std::future<void> get_return_object() noexcept
        {
            return this->get_future();
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() const noexcept { return {}; }

        void return_void() noexcept
        {
            this->set_value();
        }
        void unhandled_exception() noexcept
        {
            this->set_exception(std::current_exception());
        }
    };
};