#include <condition_variable>
#include <stop_token>
#include <thread>
#include <future>
#include <mutex>

#include <functional>
#include <queue>

#include <iostream>

class ThreadPool
{

public:

    ThreadPool(unsigned int const slavesCount) noexcept
    {
        stopSignal = std::make_shared<std::stop_source>();
        stopToken  = std::make_shared<std::stop_token >(stopSignal->get_token());
        for(unsigned int i = 0u; i < slavesCount; ++i)
        {
            slaves.emplace_back
            (
                [this](std::stop_token token) noexcept
                {
                    while(!token.stop_requested())
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock ulock{mtx};
                            cond.wait(ulock, [&]{return !taskQueue.empty() || token.stop_requested();});

                            if(taskQueue.empty() && token.stop_requested())
                               return;

                            task = std::move(taskQueue.front());   
                            taskQueue.pop();
                        }
                        task();
                    }
                },
                *stopToken
            );
        }
    }

    ~ThreadPool()
    {
        stopSignal->request_stop();
    }

    template<typename F, typename... Args>
    auto addTaskGetFuture(F &&f, Args&&... args) noexcept 
         -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using resType = std::result_of<F(Args...)>::type;
        auto task = std::packaged_task<resType()>
        (
            std::bind(static_cast<F &&>(f), static_cast<Args &&>(args)...)
        );

        std::future<resType> res = task.get_future();

        {
            std::unique_lock ulock{mtx};
            taskQueue.emplace([&task]() noexcept {task();});
        }

        cond.notify_one();
        return res;
    }

private:
    
    std::queue<std::function<void()>> taskQueue;

    std::vector<std::jthread> slaves;
    std::condition_variable cond;
    std::shared_ptr<std::stop_source> stopSignal;
    std::shared_ptr<std::stop_token>  stopToken;
    std::mutex mtx;
};


int main()
{
    unsigned int const threadCount = 7;
    ThreadPool thrPool(threadCount);

    for(unsigned int i = 0u; i < threadCount; ++i)
    {
        auto res = thrPool.addTaskGetFuture
                 (
                     [&i](int a, int b) noexcept { return a + b + i; }, 
                     7, 
                     7 * i
                 );
        std::cout << "Res: " << res.get() << std::endl;
    }

    return 0;
}
