#pragma once

#include "fakeftp.h"
#include "coroutine_traits.h"
#include "async_awaiter.h"
#include "lambda_awaiter.h"

#include <future>
#include <coroutine>

namespace kw
{
    class FakeFTP_Coroutines final : public FakeFTP
    {
    public:
        std::future<std::string>
        downloadFirstMatch(const std::string& remotePath, 
                           std::function<bool(std::string_view)>&& predicate,
                           std::function<void(int)>&& onProgress)
        {
            std::cout << "FakeFTP_Coroutines::downloadFirstMatch thread: " << std::this_thread::get_id() << '\n';
            std::vector<RemoteDirEntry> files = co_await [&]
            {
                return listFiles(remotePath);
            };

            RemoteDirEntry match = co_await [&]
            {
                return findMatchingFile(files, predicate);
            };
            
            std::string file = co_await [&]
            {
                return downloadFile(match, onProgress);
            };

            co_return file;
        }
    };
}