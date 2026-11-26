#ifndef POSIX_THREAD_POOL_H
#define POSIX_THREAD_POOL_H

#include "posix_types.h"
#include "posix_future.h"

#include <vector>
#include <queue>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <cstring>

class PosixThreadPool {
public:
    explicit PosixThreadPool(size_t numThreads) : stop(false) {
        if (numThreads == 0) numThreads = 1;

        posix_mutex_init(&queueMutex);
        posix_cond_init(&queueCond);

        for (size_t i = 0; i < numThreads; ++i) {
            PosixThread thread;
            int rc = pthread_create(&thread, nullptr,
                                    &PosixThreadPool::workerThreadProc,
                                    this);
            if (rc != 0) {
                std::cerr << "pthread_create failed: " << strerror(rc) << std::endl;
                throw std::runtime_error("pthread_create failed");
            }
            workers.push_back(thread);
        }
    }

    PosixFuture enqueue(std::function<double()> task) {
        PosixFuture future;

        auto wrappedTask = [task, future]() {
            double result = task();
            future.set(result);
        };

        posix_mutex_lock(&queueMutex);
        if (stop) {
            posix_mutex_unlock(&queueMutex);
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.push(std::move(wrappedTask));
        posix_cond_signal(&queueCond);
        posix_mutex_unlock(&queueMutex);

        return future;
    }

    ~PosixThreadPool() {
        posix_mutex_lock(&queueMutex);
        stop = true;
        posix_cond_broadcast(&queueCond);
        posix_mutex_unlock(&queueMutex);

        for (PosixThread thread : workers) {
            pthread_join(thread, nullptr);
        }

        posix_cond_destroy(&queueCond);
        posix_mutex_destroy(&queueMutex);
    }

    size_t getThreadCount() const { return workers.size(); }

private:
    std::vector<PosixThread> workers;
    std::queue<std::function<void()>> tasks;
    PosixMutex queueMutex;
    PosixCondVar queueCond;
    bool stop;

    static void* workerThreadProc(void* arg) {
        PosixThreadPool* pool = static_cast<PosixThreadPool*>(arg);
        pool->workerLoop();
        return nullptr;
    }

    void workerLoop() {
        while (true) {
            std::function<void()> task;

            posix_mutex_lock(&queueMutex);

            while (!stop && tasks.empty()) {
                posix_cond_wait(&queueCond, &queueMutex);
            }

            if (stop && tasks.empty()) {
                posix_mutex_unlock(&queueMutex);
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
            posix_mutex_unlock(&queueMutex);

            task();
        }
    }
};

#endif  