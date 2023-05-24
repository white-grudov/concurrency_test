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

    mutable handle_type m_handle;

    Task(handle_type handle)
        : m_handle(handle) 
    {}

    Task(Task&& other) noexcept 
        : m_handle(other.m_handle)
    { other.m_handle = nullptr; };

    bool await_ready() 
    { return m_handle.done(); }

    bool await_suspend(std::coroutine_handle<> handle) 
    {
        if (!m_handle.done()) {
            m_handle.resume();
        }

        return false;
    }

    auto await_resume() 
    { return get(); }

    T get() const 
    {     
        if (!m_handle.done())
            m_handle.resume();  

        if (m_handle.promise().m_exception)
            std::rethrow_exception(m_handle.promise().m_exception);

        return *m_handle.promise().m_value;
    }

    ~Task() 
    {
        if (m_handle)
            m_handle.destroy();
    }

    template<typename U>
    struct task_promise 
    {
        std::optional<T>    m_value {};
        std::exception_ptr  m_exception = nullptr;

        auto initial_suspend() 
        { return std::suspend_always{}; }

        auto final_suspend() noexcept
        { return std::suspend_always{}; }

        auto return_value(T t) 
        {
            m_value = t;
            return std::suspend_always{};
        }

        Task<T> get_return_object()
        { return {handle_type::from_promise(*this)}; }

        void unhandled_exception() 
        {  m_exception = std::current_exception(); }

        void rethrow_if_unhandled_exception()
        {
            if (m_exception)
                std::rethrow_exception(std::move(m_exception));
        }
    };

};