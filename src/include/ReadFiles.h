#pragma once

// #include <rpp/future.h>

#include <filesystem>
#include <string>
#include <vector>
#include <stdexcept>
#include <ranges>
#include <algorithm>
#include <future>

namespace fs = std::filesystem;
using FileList = std::vector<fs::path>;

class FileReader {

    std::string directoryPath;
    FileList files;

public:
    FileReader(std::string directoryPath) : directoryPath(directoryPath) {}

    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;
    FileReader(FileReader&&) = delete;
    FileReader& operator=(FileReader&&) = delete;

    FileList readFilesInDirectory() const
    {
        fs::path path(directoryPath);

        if (!fs::exists(path) || !fs::is_directory(path))
        {
            throw std::invalid_argument("Invalid directory path");
        }

        FileList files;
        auto is_regular_filter = [](const auto& entry) {
            return fs::is_regular_file(entry);
        };

        std::ranges::copy_if(fs::directory_iterator(path), 
                             std::back_inserter(files),
                             is_regular_filter);
        
        return files;
    }

    static FileList filterFiles(const FileList& files, 
                                const std::string& extension)
    {
        auto filteredFiles = files | std::views::filter([&extension](const auto& file) 
        {
            return file.extension() == extension;
        });

        return FileList(filteredFiles.begin(), filteredFiles.end());
    }
};

FileList readFilesTaskBased(const std::vector<std::string>& directories,
                            const std::vector<std::string>& extensions)
{
    std::vector<std::future<FileList>> futures;
    FileList filteredFiles;

    for (const auto& directory : directories)
    {
        std::future<FileList> future = std::async(std::launch::async, [directory]() 
        {
            FileReader reader(directory);
            return reader.readFilesInDirectory();
        });

        futures.emplace_back(std::move(future));
    }

    std::vector<FileList> resultVectors;

    for (auto& future : futures)
    {
        FileList files = future.get();
        resultVectors.emplace_back(files);
    }

    std::vector<std::future<FileList>> filterFutures;

    for (const auto& files : resultVectors)
    {
        for (const auto& extension : extensions)
        {
            std::future<FileList> filterFuture = std::async(std::launch::async,
                [files, extension]() {
                    return FileReader::filterFiles(files, extension);
                });

            filterFutures.emplace_back(std::move(filterFuture));
        }
    }

    for (auto& future : filterFutures)
    {
        FileList filteredFilesPart = future.get();
        filteredFiles.insert(filteredFiles.end(), filteredFilesPart.begin(), filteredFilesPart.end());
    }

    return filteredFiles;
}

FileList readFilesComposableFutures(const std::vector<std::string>& directories,
                                    const std::vector<std::string>& extensions)
{
    std::vector<std::future<FileList>> futures;
    FileList filteredFiles;

    for (const auto& directory : directories)
    {
        std::future<FileList> future = std::async(std::launch::async, [directory]() 
        {
            FileReader reader(directory);
            return reader.readFilesInDirectory();
        });

        futures.emplace_back(std::move(future));
    }

    // TODO: implement composable futures (rpp::cfuture)

    /*
    for (auto& future : futures)
    {
        future.then([extensions](std::future<FileList> filesFuture) 
        {
            FileList files = filesFuture.get();
            std::vector<std::future<FileList>> filterFutures;

            for (const auto& extension : extensions)
            {
                std::future<FileList> filterFuture = std::async(std::launch::async,
                    [files, extension]() {
                        return FileReader::filterFiles(files, extension);
                    });

                filterFutures.emplace_back(std::move(filterFuture));
            }

            for (auto& filterFuture : filterFutures)
            {
                filterFuture.then([&filteredFiles](std::future<FileList> filteredFilesPartFuture) 
                {
                    FileList filteredFilesPart = filteredFilesPartFuture.get();
                    filteredFiles.insert(filteredFiles.end(), filteredFilesPart.begin(), filteredFilesPart.end());
                });
            }
        });
    }
    */

    return filteredFiles;
}
