#pragma once

#include <coroutine>
#include <optional>

template <typename T>
struct Task 
{
    template<typename U>
    struct task_promise;

    using promise_type = task_promise<T>;
    using handle_type = std::coroutine_handle<promise_type>;

    mutable handle_type handle;

    Task(handle_type h) : handle { h } {}

    Task(Task&& other) noexcept : handle { std::exchange(other.handle, nullptr) } {}

    bool await_ready() noexcept { return handle.done(); }

    bool await_suspend(std::coroutine_handle<> h) noexcept
    {
        if (!handle.done())
        {
            handle.resume();
        }

        return false;
    }

    auto await_resume() { return get(); }

    T get() const 
    {     
        if (!handle.done())
        {
            handle.resume();  
        }
        auto& promise = handle.promise();

        if (promise.exception)
        {
            std::rethrow_exception(promise.exception);
        }
        return std::move(*promise.value);
    }

    ~Task() 
    {
        if (handle)
        {
            handle.destroy();
        }
    }

    template<typename U>
    struct task_promise 
    {
        std::optional<T> value;
        std::exception_ptr exception;

        auto initial_suspend() noexcept { return std::suspend_always{}; }

        auto final_suspend() noexcept { return std::suspend_always{}; }

        auto return_value(T t) noexcept
        {
            value = std::move(t);
            // return std::suspend_always{};
        }

        Task<T> get_return_object() noexcept { return {handle_type::from_promise(*this)}; }

        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
        }

        void rethrow_if_unhandled_exception()
        {
            if (exception)
            {
                std::rethrow_exception(std::move(exception));
            }
        }
    };
};
