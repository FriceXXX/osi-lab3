#ifndef POSIX_TYPES_H
#define POSIX_TYPES_H

#include <cstdint>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

using PosixMutex = pthread_mutex_t;
using PosixCondVar = pthread_cond_t;
using PosixThread = pthread_t;
using PosixThreadId = pid_t;

inline void posix_mutex_init(PosixMutex* m) {
    if (pthread_mutex_init(m, nullptr) != 0) {
        perror("pthread_mutex_init failed");
        exit(EXIT_FAILURE);
    }
}

inline void posix_mutex_destroy(PosixMutex* m) {
    pthread_mutex_destroy(m);
}

inline void posix_mutex_lock(PosixMutex* m) {
    pthread_mutex_lock(m);
}

inline void posix_mutex_unlock(PosixMutex* m) {
    pthread_mutex_unlock(m);
}

inline void posix_cond_init(PosixCondVar* c) {
    if (pthread_cond_init(c, nullptr) != 0) {
        perror("pthread_cond_init failed");
        exit(EXIT_FAILURE);
    }
}

inline void posix_cond_destroy(PosixCondVar* c) {
    pthread_cond_destroy(c);
}

inline void posix_cond_wait(PosixCondVar* c, PosixMutex* m) {
    pthread_cond_wait(c, m);
}

inline void posix_cond_signal(PosixCondVar* c) {
    pthread_cond_signal(c);
}

inline void posix_cond_broadcast(PosixCondVar* c) {
    pthread_cond_broadcast(c);
}

inline PosixThreadId posix_thread_id() {
    return syscall(SYS_gettid);
}

inline uint32_t posix_process_id() {
    return getpid();
}

inline unsigned int posix_hardware_threads() {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return (n > 0) ? static_cast<unsigned int>(n) : 1;
}

#endif