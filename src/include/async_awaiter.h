#pragma once

#include <coroutine>
#include <future>
#include <thread>

#include "thread_pool.h"

template <typename T>
struct awaiter 
{
    std::future<T>& future;

    bool await_ready() const 
    {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    void await_suspend(std::coroutine_handle<> coroutine) 
    {
        std::jthread([coroutine, &future = future]() mutable 
        {
            future.wait();
            coroutine.resume();
        }).detach();
    }

    T await_resume() 
    {
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
            coroutine.resume();
        }).detach();
    }

    void await_resume() 
    {
        return future.get();
    }
};

template <typename T>
awaiter<T> operator co_await(std::future<T> future) 
{
    return awaiter<T>{future};
}