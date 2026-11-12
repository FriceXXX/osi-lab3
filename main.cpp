#include "matrix.h"
#include "thread_pool.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#ifdef _WIN32
    #include <windows.h>
    inline DWORD getMyPid() { return GetCurrentProcessId(); }
    inline DWORD getMyTid() { return GetCurrentThreadId(); }
#else
    #include <unistd.h>
    #include <sys/syscall.h>
    inline pid_t getMyPid() { return getpid(); }
    inline pid_t getMyTid() { return syscall(SYS_gettid); }
#endif

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  -t, --threads <N>    Maximum number of threads (default: 1)\n"
              << "  -s, --size <N>       Matrix size NxN (default: 8)\n"
              << "  -f, --file <path>    Read matrix from file\n"
              << "  -h, --help           Show this help\n";
}

int main(int argc, char* argv[]) {
    size_t maxThreads = 1;
    size_t matrixSize = 8;
    std::string inputFile;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-t" || arg == "--threads") && i + 1 < argc) {
            maxThreads = std::stoul(argv[++i]);
            if (maxThreads == 0) maxThreads = 1;
        } else if ((arg == "-s" || arg == "--size") && i + 1 < argc) {
            matrixSize = std::stoul(argv[++i]);
            if (matrixSize == 0) matrixSize = 1;
        } else if ((arg == "-f" || arg == "--file") && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }
    
    std::cout << "[Main PID=" << getMyPid() << ", TID=" << getMyTid() << "] "
              << "Starting. Max threads: " << maxThreads 
              << ", Matrix size: " << matrixSize << std::endl;
    
    Matrix matrix;
    try {
        if (!inputFile.empty()) {
            matrix = Matrix::readFromFile(inputFile);
        } else {
            matrix = Matrix::generateRandom(matrixSize);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "[Main PID=" << getMyPid() << ", TID=" << getMyTid() << "] "
              << "Matrix loaded: " << matrix.size() << "x" << matrix.size() << std::endl;
    std::cout << "[Main PID=" << getMyPid() << ", TID=" << getMyTid() << "] "
              << "Hardware concurrency: " << std::thread::hardware_concurrency() << std::endl;
    
    ThreadPool pool(maxThreads);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    double det = 0.0;
    try {
        det = matrix.determinant(pool, maxThreads);
    } catch (const std::exception& e) {
        std::cerr << "Computation error: " << e.what() << std::endl;
        return 1;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "[Main PID=" << getMyPid() << ", TID=" << getMyTid() << "] "
              << "Determinant = " << det << std::endl;
    std::cout << "[Main PID=" << getMyPid() << ", TID=" << getMyTid() << "] "
              << "Computation time: " << duration.count() << " ms" << std::endl;
    std::cout << "[Main PID=" << getMyPid() << ", TID=" << getMyTid() << "] "
              << "Thread pool size: " << maxThreads << std::endl;
    
    return 0;
}