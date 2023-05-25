#include "gtest/gtest.h"

#include <string>

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

std::future<void> waitAndChangeValue(int& value)
{
    co_await std::chrono::seconds(1);
    value = 5;
}

TEST(CoroutineTest, WaitAndChangeValue)
{
    int value = 0;

    auto f = waitAndChangeValue(value);
    EXPECT_EQ(0, value);

    f.get();
    EXPECT_EQ(5, value);
}

std::future<std::string> severalLambdas()
{
    std::string result = co_await [&] { return std::string("first "); };
    co_await [&] { result += "second "; return ""; };
    auto last = co_await [&] { result += "third "; return std::string("fourth"); };

    co_return result + last;
}

TEST(CoroutineTest, SeveralLambdas)
{
    EXPECT_EQ("first second third fourth", severalLambdas().get());
}

std::future<int> exceptionHandling()
{
    int initial = 0;
    try
    {
        co_await coroutineThrowException();
        initial = 2;
    }
    catch (const std::runtime_error&)
    {
        initial = 5;
    }
    co_return initial;
}

TEST(CoroutineTest, ExceptionHandling)
{
    EXPECT_EQ(5, exceptionHandling().get());
}