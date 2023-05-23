#pragma once

// #include <rpp/future.h>
#include "FileReader.h"
#include "Generator.h"

#include <filesystem>
#include <string>
#include <vector>
#include <future>

namespace fs = std::filesystem;
using FileList = std::vector<fs::path>;

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

Generator<FileList> readFilesCoroutine(const std::vector<std::string>& directories, const std::vector<std::string>& extensions)
{
    for (const auto& directory : directories) 
    {
        FileReader reader(directory);

        for (const auto& extension : extensions) 
        {
            co_yield FileReader::filterFiles(reader.readFilesInDirectory(), extension);
        }
    }
}

FileList readFilesCoroutineBased(const std::vector<std::string>& directories, const std::vector<std::string>& extensions)
{
    FileList filteredFiles;
    auto generator = readFilesCoroutine(directories, extensions);

    for (int i = 0; generator; ++i) 
    {
        FileList files = generator();
        filteredFiles.insert(filteredFiles.end(), files.begin(), files.end());
    }

    return filteredFiles;
}