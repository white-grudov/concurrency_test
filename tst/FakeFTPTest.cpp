#include "gtest/gtest.h"

#include <string>
#include <stdexcept>
#include <future>

#include "fakeftp_coroutines.h"

TEST(FakeFTPTest, FileDownloadSuccessful)
{
    kw::FakeFTP_Coroutines ftp;
    const std::string path = "/home/whitegrudov/test";
    const std::string ext = ".exe";

    auto file = ftp.downloadFirstMatch(path,
        [&ext](std::string_view f) { return f.ends_with(ext); },
        [](int progress) { return; });

    EXPECT_EQ("/tmp/test.exe", file.get());
}

TEST(FakeFTPTest, FileDownloadFailed)
{
    kw::FakeFTP_Coroutines ftp;
    const std::string path = "/home/whitegrudov/test";
    const std::string ext = ".dll";

    try
    {
        auto file = ftp.downloadFirstMatch(path,
            [&ext](std::string_view f) { return f.ends_with(ext); },
            [](int progress) { return; });
        file.get();
        FAIL();
    }
    catch (const std::exception& e)
    {
        EXPECT_STREQ("FTP no files matched the search pattern", e.what());
    }
}