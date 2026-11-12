#include "matrix.h"
#include "thread_pool.h"
#include <stdexcept>
#include <fstream>
#include <random>
#include <cmath>

Matrix::Matrix(size_t n, const std::vector<double>& data) 
    : n(n), data(data) {
    if (n == 0) {
        throw std::invalid_argument("Matrix size must be > 0");
    }
    if (data.size() != n * n) {
        throw std::invalid_argument("Invalid matrix size: expected " + 
                                   std::to_string(n * n) + " elements, got " + 
                                   std::to_string(data.size()));
    }
}

Matrix Matrix::getMinor(size_t row, size_t col) const {
    if (n <= 1) {
        throw std::invalid_argument("Cannot get minor of 1x1 matrix");
    }
    
    size_t newSize = n - 1;
    std::vector<double> minorData;
    minorData.reserve(newSize * newSize);
    
    for (size_t i = 0; i < n; ++i) {
        if (i == row) continue;
        for (size_t j = 0; j < n; ++j) {
            if (j == col) continue;
            minorData.push_back(data[i * n + j]);
        }
    }
    
    return Matrix(newSize, minorData);
}

double Matrix::determinantSequential() const {
    if (n == 1) return data[0];
    if (n == 2) return data[0] * data[3] - data[1] * data[2];
    
    double det = 0.0;
    for (size_t j = 0; j < n; ++j) {
        double cofactor = (j % 2 == 0 ? 1.0 : -1.0) * data[j];
        det += cofactor * getMinor(0, j).determinantSequential();
    }
    return det;
}

double Matrix::determinant(ThreadPool& pool, size_t maxThreads, size_t depth) const {
    if (maxThreads <= 1 || n < MIN_SIZE_FOR_PARALLEL || depth >= MAX_PARALLEL_DEPTH) {
        return determinantSequential();
    }
    
    if (n == 1) return data[0];
    if (n == 2) return data[0] * data[3] - data[1] * data[2];
    
    std::vector<std::future<double>> futures;
    futures.reserve(n);
    
    for (size_t j = 0; j < n; ++j) {
        double cofactor = (j % 2 == 0 ? 1.0 : -1.0) * data[j];
        Matrix minor = getMinor(0, j);
        
        auto minorPtr = std::make_shared<Matrix>(std::move(minor));
        
        futures.push_back(pool.enqueue(
            [cofactor, minorPtr, &pool, maxThreads, depth]() {
                return cofactor * minorPtr->determinant(pool, maxThreads, depth + 1);
            }
        ));
    }
    
    double det = 0.0;
    for (auto& f : futures) {
        det += f.get();
    }
    
    return det;
}

Matrix Matrix::generateRandom(size_t n) {
    if (n == 0) n = 1;
    
    std::vector<double> data(n * n);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-10.0, 10.0);
    
    for (auto& val : data) {
        val = dis(gen);
    }
    
    return Matrix(n, data);
}

Matrix Matrix::readFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    size_t n;
    file >> n;
    
    if (n == 0) {
        throw std::runtime_error("Invalid matrix size in file: 0");
    }
    
    std::vector<double> data(n * n);
    for (size_t i = 0; i < n * n; ++i) {
        if (!(file >> data[i])) {
            throw std::runtime_error("Failed to read matrix data from file");
        }
    }
    
    return Matrix(n, data);
}