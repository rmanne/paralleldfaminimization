/* @file Mutex.h
 *
 * This class is just a wrapper around std::mutex.
 */
#pragma once

#include <pthread.h>

class Mutex {
private:
    pthread_mutex_t m;

public:
    Mutex() {
        pthread_mutex_init(&m, NULL);
    }

    void lock() {
        pthread_mutex_lock(&m);
    }

    void unlock() {
        pthread_mutex_unlock(&m);
    }
};
