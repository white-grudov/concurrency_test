#include <vector>
#include <string>
#include <string_view>
#include <cstddef> // size_t
#include <functional> // std::function
#include <filesystem>
#include <fstream>
#include <cstdio>

#include "rpp/future.h"

namespace kw
{
    // both ground control software and other utils use printf style logging since it's faster
    #define LogInfo(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
    #define LogError(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

    namespace fs = std::filesystem;

    class ComposableFTPExample
    {
        // spec: last LIST result needs to be kept around for the UI
        //       this complicates the implementation
        std::vector<RemoteDirEntry> listed;
        std::string listedPath;

    public:

        ComposableFTPExample() noexcept = default;

        /** @returns List of remote dir entries from the last `listFiles` call, for the UI */
        const std::vector<RemoteDirEntry>& getListed() const noexcept { return listed; }

        /** @returns Listed remote path name from the last `listFiles` call, for the UI */
        const std::string& getListedPath() const noexcept { return listedPath; }

        /**
         * @brief Downloads the first file that matches the predicate
         * @param remotePath Remote path to fetch LIST of files from
         * @param predicate Files filter to select the file (convoluted extra step)
         * @param onProgress Progress report callback for the UI progress bar
         * @returns Local temp path of the downloaded file
         */
        rpp::cfuture<std::string> downloadFirstMatch(const std::string& remotePath, 
                                                     std::function<bool(std::string_view)> predicate,
                                                     std::function<void(int)> onProgress)
        {
            rpp::cfuture<std::vector<RemoteDirEntry>> filesFuture = 
                std::async(std::launch::async, [this, remotePath]{
                    return listFiles(remotePath);
            });

            return filesFuture.then([this, predicate](const std::vector<RemoteDirEntry>& files) {
                return findMatchingFile(files, std::move(predicate));
            }).then([this, onProgress](const RemoteDirEntry& match) {
                return downloadFile(match, std::move(onProgress));
            });
        }

    private:

        std::vector<RemoteDirEntry> listFiles(const std::string& remotePath)
        {
            LogInfo("LIST %s", remotePath.c_str());

            if (!fs::exists(remotePath)) // failures are handled by exceptions
                throw std::runtime_error{"FTP remote path does not exist: " + remotePath};

            std::vector<RemoteDirEntry> list;
            for (const fs::directory_entry& dirEntry : fs::directory_iterator{remotePath})
            {
                list.emplace_back(dirEntry.path().string(), dirEntry.file_size(), dirEntry.is_regular_file());
                const RemoteDirEntry& e = list.back();
                if (e.isFile) LogInfo("  file %s (%zu KB)", e.path(), e.size);
                else          LogInfo("  dir  %s", e.path());
            }

            listed = list; // make a copy for the UI to use later
            listedPath = remotePath;
            return list;
        }

        static RemoteDirEntry findMatchingFile(const std::vector<RemoteDirEntry>& list,
                                               std::function<bool(std::string_view)> predicate)
        {
            for (auto& e : list)
                if (e.isFile && predicate(e.remotePath))
                    return e;
            throw std::runtime_error{"FTP no files matched the search pattern"};
        }

        std::string downloadFile(const RemoteDirEntry& remoteFile,
                                 std::function<void(int)> onProgress)
        {
            LogInfo("DOWNLOAD %s (%zu KB)", remoteFile.path(), remoteFile.size / 1024);
            if (!remoteFile.isFile)
                throw std::runtime_error{"FTP download failed, not a file: " + remoteFile.remotePath};
            
            std::ifstream inFile { remoteFile.remotePath, std::ios::binary };
            if (!inFile)
                throw std::runtime_error{"FTP download request failed: " + remoteFile.remotePath};

            std::string tempPath = (fs::temp_directory_path() / fs::path{remoteFile.remotePath}.filename()).string();
            std::ofstream outFile { tempPath, std::ios::binary };
            if (!outFile)
                throw std::runtime_error{"FTP failed to create temp file at: " + tempPath};
            
            // perform a "fake download"
            int prevProgress = -1;
            char buf[128]; // artificially small buffer for this fake example
            for (size_t i = 0; i < remoteFile.size; ++i)
            {
                inFile.read(buf, sizeof(buf));
                size_t bytesRead = inFile.gcount();
                outFile.write(buf, bytesRead);

                i += bytesRead;

                // report progress to the UI
                if (int progress = static_cast<int>((i * 100) / remoteFile.size); prevProgress != progress)
                {
                    prevProgress = progress;
                    onProgress(progress); // the UI will handle synchronization
                }
            }
            return tempPath;
        }
    };
}