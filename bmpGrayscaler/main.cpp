#include "bitmap.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>
using namespace std;
struct Config
{
    string inputPath{};
    string outputPath{};
    int threadsCount{1};
    void print();
    ;
};

void printHelp()
{
    cout
        << "Usage: \n"
           "  bmpGrayScaler [--help|h] [-t threadCount] [-o OutputPath] <InputPath>\n"
           "Keys:\n"
           "  --help or -h\tThis message\n"
           "  -t threadCount\tCount of working threads. Default=1\n"
           "  -o outputPath\tOutput directory. Default=<$IN>/out\n"
           "  InputPath \tInput path Default=./in\n";
}

int parseArgs(int argCnt, char ** args, Config & config)
{
    if (argCnt > 1)
    {
        for (int i = 1; i < argCnt; i++)
        {
            if (i == argCnt - 1)
            {
                config.inputPath = string(args[i]);
            }
            if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
            {
                printHelp();
                return 0;
            }
            if (strcmp(args[i], "-t") == 0)
            {
                if (argCnt > i + 1)
                {
                    i++;
                    config.threadsCount = stoi(string(args[i]));
                }
            }

            if (strcmp(args[i], "-o") == 0)
            {
                if (argCnt > i + 1)
                {
                    i++;
                    config.outputPath = string(args[i]);
                }
            }
        }
    }
    if (config.inputPath.empty())
    {
        config.inputPath = "./in/";
    }
    if (config.outputPath.empty())
    {
        config.outputPath = config.inputPath;
        if (config.outputPath[config.outputPath.size() - 1] != '/')
        {
            config.outputPath += "/";
        }
        config.outputPath += "out";
    }
    return 1;
}

void Config::print()
{
    printf("Run configuration:\n  Input:\t\t%s\n  Output:\t\t%s\n  Threads count:\t%d\n\n",
        this->inputPath.data(), this->outputPath.data(), this->threadsCount);
}

int main(int argCnt, char ** args)
{
    Config config;
    if (!parseArgs(argCnt, args, config))
    {
        return 0;
    }
    config.print();

    using Task = std::function<void()>;
    vector<Task> tasks;
    std::atomic<int> lastTask{0};
    auto start = chrono::high_resolution_clock::now();
    vector<thread> threads;
    ::filesystem::path inputDir(config.inputPath);
    if (::filesystem::is_directory(inputDir))
    {
        ::filesystem::path outputDir(config.outputPath);
        if (!::filesystem::exists(outputDir))
        {
            ::filesystem::create_directories(outputDir);
            cout << config.outputPath << " created!\n";
            flush(cout);
        }
        else if (::filesystem::exists(outputDir) && !::filesystem::is_directory(outputDir))
        {
            cout << config.outputPath << " exists and not a directory!\n";
            return 0;
        }
        for (auto const & dir_entry : std::filesystem::directory_iterator{inputDir})
        {
            string filename = dir_entry.path().string();
            if (filename.find(".bmp") != string::npos)
            {
                string outFilename = outputDir.string() + "/" + dir_entry.path().filename().string();
                tasks.emplace_back([filename, outFilename] {
                    FILE * file = fopen(filename.data(), "rb");
                    if (file == nullptr)
                    {
                        return;
                    }
                    fseek(file, 0, SEEK_END);
                    long size = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    char * buffer = new char[size];
                    fread(buffer, 1, size, file);

                    fclose(file);

                    BMP::Bitmap bmp(buffer, size);
                    if (bmp.isValid())
                    {
                        bmp.makeBW();
                    }

                    {
                        FILE * file = fopen(outFilename.data(), "wb");
                        if (file == nullptr)
                        {
                            return;
                        }
                        fwrite(buffer, 1, size, file);

                        fclose(file);
                    }
                });
            }
        }
    }else{
        cout << config.inputPath << " is not a directory!\n";
    }
    threads.reserve(config.threadsCount);
    for (int i = 0; i < config.threadsCount; i++)
    {
        threads.emplace_back(thread([&tasks, &lastTask, i] {
            while (true)
            {
                int taskInd = ++lastTask;
                if (taskInd < tasks.size())
                {
                    tasks[taskInd]();
                }
                else
                {
                    printf("Thread #%d finished\n", i);
                    return;
                }
            }
        }));
    }
    for (auto & t : threads)
    {
        t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    cout << float(dur.count()) / 1000000. << endl;

    return 0;
}
