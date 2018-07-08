#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <future>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>

class thread_pool
{
public:
    thread_pool(size_t sz);
    ~thread_pool();
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
private:
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _tasks;

    std::mutex _sync_queue;
    std::condition_variable _condition;
    bool stop;
};

inline thread_pool::thread_pool(size_t sz)
: stop(false)
{
    for(size_t i = 0; i < sz; ++i)
    {
        _threads.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->_sync_queue);
                        this->_condition.wait(
                            lock,
                            [this] { return this->stop || !this->_tasks.empty(); }
                        );
                        if(this->stop && this->_tasks.empty())
                            return;
                        task = std::move(this->_tasks.front());
                        this->_tasks.pop();
                    }
                    task();
                }
            }
        );
    }
}

inline thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lock(_sync_queue);
        stop = true;
    }
    _condition.notify_all();
    for(std::thread& w : _threads)
    {
        w.join();
    }
}

template<typename F, typename... Args>
auto thread_pool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_t = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_t()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_t> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(_sync_queue);
        if(stop)
            return res;
        _tasks.emplace([task](){(*task)();});
    }
    _condition.notify_one();
    return res;
}

#endif
