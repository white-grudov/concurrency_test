#pragma once

#include <coroutine>
#include <future>
#include <thread>

template <typename T>
struct awaiter 
{
    std::future<T>& future;
    std::exception_ptr exception;

    bool await_ready() const 
    {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void await_suspend(std::coroutine_handle<> coroutine) 
    {
        std::jthread([coroutine, &future = future, &exception = exception]() mutable 
        {
            try 
            {
                future.wait();
                coroutine();
            } 
            catch (...) 
            {
                exception = std::current_exception();
                coroutine();
            }
        }).detach();
    }

    T await_resume() 
    {
        if (exception) 
        {
            std::rethrow_exception(exception);
        }
        return future.get();
    }
};

template <>
struct awaiter<void> 
{
    std::future<void>& future;

    bool await_ready() const 
    {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void await_suspend(std::coroutine_handle<> coroutine) 
    {
        std::jthread([coroutine, &future = future]() mutable 
        {
            future.wait();
            coroutine();
        }).detach();
    }

    void await_resume() 
    {
        future.get();
    }
};

template <typename T>
awaiter<T> operator co_await(std::future<T> future) 
{
    return awaiter<T>{future};
}