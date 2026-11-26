#ifndef POSIX_FUTURE_H
#define POSIX_FUTURE_H

#include "posix_types.h"
#include <memory>

class PosixFuture {
public:
    PosixFuture() : state(std::make_shared<FutureState>()) {}

    PosixFuture(const PosixFuture& other) : state(other.state) {}
    PosixFuture(PosixFuture&& other) noexcept : state(std::move(other.state)) {}

    void set(double result) const {
        posix_mutex_lock(&state->mutex);
        state->value = result;
        state->ready = true;
        posix_cond_signal(&state->cond);
        posix_mutex_unlock(&state->mutex);
    }

    double get() const {
        posix_mutex_lock(&state->mutex);
        while (!state->ready) {
            posix_cond_wait(&state->cond, &state->mutex);
        }
        double result = state->value;
        posix_mutex_unlock(&state->mutex);
        return result;
    }

private:
    struct FutureState {
        PosixMutex mutex;
        PosixCondVar cond;
        bool ready = false;
        double value = 0.0;

        FutureState() {
            posix_mutex_init(&mutex);
            posix_cond_init(&cond);
        }
        ~FutureState() {
            posix_cond_destroy(&cond);
            posix_mutex_destroy(&mutex);
        }
        FutureState(const FutureState&) = delete;
        FutureState& operator=(const FutureState&) = delete;
    };

    std::shared_ptr<FutureState> state;
};

#endif