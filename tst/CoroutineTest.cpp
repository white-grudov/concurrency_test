#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <thread>

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

TEST(CoroutineTest, StartsOnAwait)
{
    bool started = false;

    auto f = [&]() -> std::future<void>
    {
        started = true;
        co_return;
    };

    [&]() -> std::future<void>
    {
        EXPECT_FALSE(started);
        co_await f();
        EXPECT_TRUE(started);
    }().get();
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
    result += co_await [&] { return "second "; };
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

std::future<void> voidLambdas(std::string& value, const std::string& newValue)
{
    co_await std::chrono::seconds(1);
    co_await [&] { value = newValue; };
}

TEST(CoroutineTest, VoidLambdas)
{
    std::string value = "initial";
    auto f = voidLambdas(value, "new");
    EXPECT_EQ("initial", value);

    f.get();
    EXPECT_EQ("new", value);
}

std::future<std::vector<std::thread::id>> severalThreads()
{
    std::vector<std::thread::id> ids;

    ids.emplace_back(co_await []() -> std::thread::id { return std::this_thread::get_id(); });
    ids.emplace_back(co_await []() -> std::thread::id { return std::this_thread::get_id(); });

    co_return ids;
}

TEST(CoroutineTest, SeveralThreads)
{
    auto ids = severalThreads().get();
    EXPECT_NE(ids[0], ids[1]);
}

std::future<std::vector<int>> destructorSequence()
{
    std::vector<int> dtorIds;
    
    struct DtorRecorder
    {
        std::vector<int>& results;
        const int id;
        ~DtorRecorder() noexcept { results.emplace_back(id); }
    };

    co_await ([&dtorIds]() -> std::future<void>
    {
        DtorRecorder dr {dtorIds, 1};
        co_await std::chrono::milliseconds{10};
    }());

    EXPECT_EQ(dtorIds.size(), 1u);
    EXPECT_EQ(dtorIds[0], 1);

    co_await ([&dtorIds]() -> std::future<void>
    {
        DtorRecorder dr {dtorIds, 2};
        co_await std::chrono::milliseconds{5};
    }());

    EXPECT_EQ(dtorIds.size(), 2u);
    EXPECT_EQ(dtorIds[1], 2);

    co_await ([&dtorIds]() -> std::future<void>
    {
        DtorRecorder dr {dtorIds, 3};
        co_return;
    }());
    
    EXPECT_EQ(dtorIds.size(), 3u);
    EXPECT_EQ(dtorIds[2], 3);

    co_return dtorIds;
}

TEST(CoroutineTest, DtorsCalledSequentially)
{
    std::vector<int> dtorIds = destructorSequence().get();

    EXPECT_EQ(dtorIds.size(), 3u);
    EXPECT_EQ(dtorIds[0], 1);
    EXPECT_EQ(dtorIds[1], 2);
    EXPECT_EQ(dtorIds[2], 3);
}