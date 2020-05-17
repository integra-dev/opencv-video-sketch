/* 
C++11
A thread-safe-queue class to store frames extracted from video.
The class is based on std::queue and implementing of mutex and std::condition_variable to wait while queue has an element to pop()/
*/

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

#pragma once

template <class T>
class ThreadSafeQueue
{
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    ThreadSafeQueue() { };
    ThreadSafeQueue(const ThreadSafeQueue& s_q) {         // custom copy constructor
        queue_ = s_q.queue_;
    };

    T pop()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        auto item = queue_.front();
        queue_.pop();
        mlock.unlock();
        return item;
    }

    void pop(T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        item = queue_.front();

        queue_.pop();
    }

    void push(T item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);

        mlock.unlock();
        cond_.notify_one();
    }

    void push(T&& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(std::move(item));
        mlock.unlock();
        cond_.notify_one();
    }

    bool is_empty()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        return queue_.empty();
    }

    size_t get_size()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        return queue_.size();
    }
};