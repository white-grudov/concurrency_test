#pragma once

#include <coroutine>
#include <future>
#include <thread>

template <typename T>
struct awaiter 
{
    std::future<T>& future;
    std::exception_ptr ex;

    bool await_ready() const 
    {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void await_suspend(std::coroutine_handle<> coroutine) 
    {
        std::jthread([coroutine, &future = future, &ex = ex]() mutable 
        {
            try
            {
                future.wait();
            }
            catch (...)
            {
                ex = std::current_exception();
            }
            coroutine.resume();
        }).detach();
    }

    T await_resume() 
    {
        if (ex) 
        {
            std::rethrow_exception(ex);
        }
        return future.get();
    }
};

template <>
struct awaiter<void> 
{
    std::future<void>& future;
    std::exception_ptr ex;

    bool await_ready() const 
    {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void await_suspend(std::coroutine_handle<> coroutine) 
    {
        std::jthread([coroutine, &future = future, &ex = ex]() mutable 
        {
            try
            {
                future.wait();
            }
            catch (...)
            {
                ex = std::current_exception();
            }
            coroutine.resume();
        }).detach();
    }

    void await_resume() 
    {
        if (ex) 
        {
            std::rethrow_exception(ex);
        }
        return future.get();
    }
};

template <typename T>
awaiter<T> operator co_await(std::future<T> future) 
{
    return awaiter<T>{future};
}