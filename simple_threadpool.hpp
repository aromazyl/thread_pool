#pragma once
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/bind.hpp>
#include "thread_safe_queue.hpp"
#include "condition_variable.hpp"

class ThreadPool {
    public:
        ThreadPool(): done_(false) {
            unsigned const thread_count = boost::thread::hardware_concurrency();
            try {
                for (unsigned i = 0; i < thread_count; ++i) {
                    threads_.push_back(new boost::thread(&ThreadPool::WorkerThread, this));
                }
            } catch (...) {
                done_ = true;
                throw;
            }
        }

        ~ThreadPool() {
            done_ = true;
            for (size_t i = 0; i < threads_.size(); ++i) {
                threads_[i]->join();
                delete threads_[i];
            }
        }

    public:
        template <typename FunctionType>
        void Submit(FunctionType f) {
            queue_.Push(boost::function<void()>(f));
            cond_.Post();
        }

    private:
        void WorkerThread() {
            while (!done_) {
                boost::function<void()> task;
                if (queue_.TryPop(task)) {
                    task();
                } else {
                    cond_.Wait();
                }
            }
        }

    private:
        boost::atomic_bool done_;
        ThreadSafeQueue<boost::function<void()> > queue_;
        ConditionVariable cond_;
        std::vector<boost::thread*> threads_;
        // join_threads joiner_;
};
