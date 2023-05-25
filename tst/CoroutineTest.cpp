#include "gtest/gtest.h"

#include "coroutine_traits.h"
#include "async_awaiter.h"
#include "lambda_awaiter.h"
#include "chrono_awaiter.h"

std::future<int> coroutineReturnValue()
{
    co_return 5;
}

TEST(CoroutineTest, ReturnValue)
{
    EXPECT_EQ(5, coroutineReturnValue().get());
}

std::future<void> coroutineSetValue(int& value)
{
    value = 5;
    co_return;
}

TEST(CoroutineTest, SetValue)
{
    int value = 0;
    coroutineSetValue(value).get();

    EXPECT_EQ(5, value);
}

std::future<void> coroutineThrowException()
{
    throw std::runtime_error("test");
    co_return;
}

TEST(CoroutineTest, ThrowException)
{
    EXPECT_THROW(coroutineThrowException().get(), std::runtime_error);
}

std::future<void> coroutineThrowExceptionInAwaiter()
{
    co_await std::async(std::launch::async, []() { throw std::runtime_error("test"); });
}

TEST(CoroutineTest, ThrowExceptionInAwaiter)
{
    EXPECT_THROW(coroutineThrowExceptionInAwaiter().get(), std::runtime_error);
}

