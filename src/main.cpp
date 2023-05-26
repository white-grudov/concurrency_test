#include "include/fakeftp_coroutines.h"

#include <string>
#include <string_view>

void showResultInUI(bool success)
{
    if (success) LogInfo("SUCCESS");
    else         LogError("FAILURE");
}

std::future<void> test_Coroutines()
{
    std::cout << "test_Coroutines thread: " << std::this_thread::get_id() << '\n';
    LogInfo("Test: Coroutines");
    kw::FakeFTP_Coroutines ftp;
    try
    {
        std::string file = co_await ftp.downloadFirstMatch("/home/whitegrudov/test",
			[](std::string_view f) { return f.ends_with(".dll"); },
	        [](int progress) { LogInfo("Download: %d%%", progress); });
        LogInfo("downloadFirstMatch success: %s", file);
        showResultInUI(true);
    }
    catch (const std::exception& e)
    {
        LogError("downloadFirstMatch failed: %s", e.what());
        showResultInUI(false);
    }
}

int main()
{
    std::cout << "main thread: " << std::this_thread::get_id() << '\n';
    test_Coroutines().wait();
    return 0;
}