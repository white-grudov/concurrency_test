#include "gtest/gtest.h"

#include <string>
#include <stdexcept>
#include <future>

#include "fakeftp_coroutines.h"

std::future<std::string> getFileName(const std::string& path, const std::string& ext)
{
    kw::FakeFTP_Coroutines ftp;
    std::string file = co_await ftp.downloadFirstMatch(path,
        [&ext](std::string_view f) { return f.ends_with(ext); },
        [](int progress) { return; });
    co_return file;
}

TEST(FakeFTPTest, FileDownloadSuccessful)
{
    kw::FakeFTP_Coroutines ftp;
    const std::string path = "/home/whitegrudov/test";
    const std::string ext = ".exe";

    auto fileFuture = getFileName(path, ext);
    auto filename = fileFuture.get();

    EXPECT_EQ("/tmp/test.exe", filename);
}

TEST(FakeFTPTest, FileDownloadFailed)
{
    kw::FakeFTP_Coroutines ftp;
    const std::string path = "/home/whitegrudov/test";
    const std::string ext = ".dll";

    try
    {
        auto fileFuture = getFileName(path, ext);
        auto filename = fileFuture.get();
        FAIL();
    }
    catch (const std::exception& e)
    {
        EXPECT_STREQ("FTP no files matched the search pattern", e.what());
    }
}