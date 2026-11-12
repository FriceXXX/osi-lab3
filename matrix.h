#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <cstddef>
#include <string>

class ThreadPool;

class Matrix {
public:
    Matrix() : n(0) {}
    Matrix(size_t n, const std::vector<double>& data);
    
    Matrix getMinor(size_t row, size_t col) const;
    double determinant(ThreadPool& pool, size_t maxThreads, size_t depth = 0) const;
    size_t size() const { return n; }
    
    static Matrix generateRandom(size_t n);
    static Matrix readFromFile(const std::string& filename);

private:
    size_t n;
    std::vector<double> data;
    double determinantSequential() const;
    
    static const size_t MAX_PARALLEL_DEPTH = 3;
    static const size_t MIN_SIZE_FOR_PARALLEL = 4;
};

#endif