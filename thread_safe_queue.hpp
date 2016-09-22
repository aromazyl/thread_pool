#pragma once
#include <queue>
#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/move/move.hpp>
#include "condition_variable.hpp"
#include "AutoLock.hpp"

template <typename T>
class ThreadSafeQueue {
    private:
        pthread_mutex_t mutex_;
        std::queue<boost::shared_ptr<T> > data_queue_;
        ConditionVariable cond_;

    public:
        ThreadSafeQueue() {
            pthread_mutex_init(&mutex_, NULL);
        }
        ~ThreadSafeQueue() {
            cond_.Post();
            pthread_mutex_destroy(&mutex_);
        }

        void WaitAndPop(T& value) {
            AutoLock(&(this->mutex_));
            cond_.Wait();
            value = boost::move(*data_queue_.front());
            data_queue_.pop();
        }

        bool TryPop(T& value) {
            AutoLock(&(this->mutex_));
            if (data_queue_.empty()) return false;
            value = boost::move(*data_queue_.front());
            data_queue_.pop();
            return true;
        }

        boost::shared_ptr<T> WaitAndPop() {
            AutoLock(&(this->mutex_));
            if (data_queue_.empty()) cond_.Wait();
            boost::shared_ptr<T> res = data_queue_.front();
            data_queue_.pop();
            return res;
        }

        boost::shared_ptr<T> TryPop() {
            AutoLock(&(this->mutex_));
            if (data_queue_.empty()) return boost::shared_ptr<T>();
            boost::shared_ptr<T> res = data_queue_.front();
            data_queue_.pop();
            return res;
        }

        void Push(T new_value) {
            boost::shared_ptr<T> data(boost::make_shared<T>(boost::move(new_value)));
            AutoLock(&(this->mutex_));
            data_queue_.push(data);
            cond_.Post();
        }

        bool Empty() const {
            AutoLock(&(this->mutex_));
            return data_queue_.empty();
        }

};
