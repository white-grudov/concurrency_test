#include <string>
#include <string_view>

#include "FTPRemoteAsync.h"
#include "FTPRemoteComposable.h"
#include "FTPRemoteCoroutines.h"
#include "Task.h"

int main()
{
	// usage example, errors must be handled
    // the actual work should be done in a background thread
    // UI holds something equivalent to a future<string> to get the final result
    try
    {
	    kw::CoroutineFTPExample ftp;
	    auto fileFuture = ftp.downloadFirstMatch("/home/whitegrudov/test",
			[](std::string_view f) { return f.ends_with(".exe"); },
	        [](int progress) { LogInfo("Download: %d%%", progress); });

        // other tasks

        std::string file = fileFuture.get();
        
        LogInfo("downloadFirstMatch success: %s", file.c_str());
    }
    catch (const std::exception& e)
    {
        LogError("downloadFirstMatch failed: %s", e.what());
    }
    return 0;
}