#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <stdexcept>
#include <ranges>
#include <algorithm>

namespace fs = std::filesystem;
using FileList = std::vector<fs::path>;

class FileReader {

    std::string directoryPath;
    FileList files;

public:
    FileReader(std::string directoryPath) noexcept : directoryPath { directoryPath } {}

    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;
    FileReader(FileReader&&) = delete;
    FileReader& operator=(FileReader&&) = delete;

    FileList readFilesInDirectory() const
    {
        fs::path path { directoryPath };

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