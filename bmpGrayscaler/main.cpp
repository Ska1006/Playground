#include "bitmap.h"

#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
using namespace std;
struct Config{
    string inputPath{};
    string outputPath{};
    int threadsCount{1};
    void print();;
};

void printHelp(){
    cout << "Usage:" << endl
         << "  bmpGrayScaler [--help|h] [-t threadCount] [-o OutputPath] <InputPath>" << endl
         << "Keys:" << endl
         << "  --help or -h\t\tThis message" << endl
         << "  -t threadCount\tCount of working threads.\tDefault=1" << endl
         << "  -o outputPath\t\tOutput directory.\t\tDefault:<$IN>/out" << endl
         << "  InputPath\t\tInput path\t\t\tDefault:./in" << endl;
}

int parseArgs(int argCnt, char** args, Config & config)
{
    if(argCnt > 1){
        for(int i = 1; i < argCnt; i++)
        {
            if(i == argCnt - 1){
                config.inputPath = string(args[i]);
            }
            if(strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0){
                printHelp();
                return 0;
            }
            if(strcmp(args[i], "-t") == 0){
                if(argCnt > i + 1){
                    i++;
                    config.threadsCount = stoi(string(args[i]));
                }
            }

            if(strcmp(args[i], "-o") == 0){
                if(argCnt > i + 1){
                    i++;
                    config.outputPath = string(args[i]);
                }
            }

        }
    }
    if(config.inputPath.empty()){
        config.inputPath = "./in/";
    }
    if(config.outputPath.empty()){
        config.outputPath = config.inputPath;
        if(config.outputPath[config.outputPath.size() - 1] != '/')
            config.outputPath += "/";
        config.outputPath += "out";

    }
    return 1;
}


void Config::print()
{
    printf( "Run configuration:\n  Input:\t\t%s\n  Output:\t\t%s\n  Threads count:\t%d\n\n",
           this->inputPath.data(), this->outputPath.data(), this->threadsCount);
}

int main(int argCnt, char** args)
{
    Config config;
    if(!parseArgs(argCnt, args, config))
        return 0;
    config.print();

    using Task = std::function<void()>;
    vector<Task> tasks;
    std::atomic<int> lastTask{0};
    auto start = chrono::high_resolution_clock::now();
    vector<thread> threads;
    ::filesystem::path inputDir(config.inputPath);
    ::filesystem::path outputDir(config.outputPath);
    ::filesystem::create_directories(outputDir);
    for (auto const& dir_entry : std::filesystem::directory_iterator{inputDir}){
        string filename = dir_entry.path().string();
        if(filename.find(".bmp") != string::npos){
            string outFilename = outputDir.string()+"/" + dir_entry.path().filename().string();
            tasks.emplace_back([filename, outFilename]{
                FILE* file = fopen(filename.data(), "rb");

                fseek(file, 0, SEEK_END);
                long size = ftell(file);
                fseek(file, 0, SEEK_SET);

                char* buffer = new char[size];
                fread(buffer, 1, size, file);

                fclose(file);

                BMP::Bitmap bmp(buffer,size);
                bmp.makeBW();

                {
                    FILE* file = fopen(outFilename.data(), "wb");

                    fwrite(buffer, 1, size, file);

                    fclose(file);
                }

                delete [] buffer;
            });
        }
    }
    threads.reserve(config.threadsCount);
    for(int i = 0; i < config.threadsCount; i++){
        threads.emplace_back(thread([&tasks, &lastTask, i]{
            while( true ) {
                int taskInd = ++lastTask;
                if (taskInd < tasks.size()){
                    printf("%d %d \n", i ,taskInd);
                    tasks[taskInd]();
                }else{
                    printf("%d end\n", i);
                    return;
                }
            }
        }));
    }
    for(auto & t : threads)
        t.join();

    auto end = chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    cout << float(dur.count())/1000000. << endl;

    return 0;
}
