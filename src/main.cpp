#include <iostream>

#include "ReadFileFuncs.h"

int main()
{
    std::vector<std::string> directories = { "/home/whitegrudov/test/1",
                                             "/home/whitegrudov/test/2",
                                             "/home/whitegrudov/test/3" };
    std::vector<std::string> extensions = { ".txt", ".png" };

    // FileList files = readFilesTaskBased(directories, extensions);
    FileList files = readFilesCoroutineBased(directories, extensions);

    for (const auto& file : files)
    {
        std::cout << file.string() << std::endl;
    }

    return 0;
}