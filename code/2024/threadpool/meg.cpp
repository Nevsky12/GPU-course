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
    : stop(false)
    {
        for(unsigned int i = 0u; i < slavesCount; ++i)
        {
            slaves.emplace_back
            (
                [this] noexcept
                {
                    for(;;)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock ulock(mtx);
                            cond.wait(ulock, [this]{return !taskQueue.empty() || stop;});

                            if(taskQueue.empty() || stop)
                               return;

                            task = std::move(taskQueue.front());   
                            taskQueue.pop();
                        }
                        task();
                    }
                }
            );
        }
    }


    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> ulock(mtx);
            stop = true;
        }
        cond.notify_all();
    }

    template<typename F, typename... Args>
    auto addTaskSeeFuture(F &&f, Args&&... args) noexcept 
         -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using resType = std::invoke_result<F, Args...>::type;
        auto task = std::make_shared<std::packaged_task<resType()>>
        (
            std::bind(static_cast<F &&>(f), static_cast<Args &&>(args)...)
        );
        std::future<resType> res = task->get_future();

        {
            std::unique_lock ulock{mtx};

            taskQueue.emplace
            (
                [task] noexcept { (*task)(); }
            );
        }

        cond.notify_one();
        return res;
    }

private:
    
    std::queue<std::function<void()>> taskQueue;

    std::vector<std::jthread> slaves;
    std::condition_variable cond;
    std::mutex mtx;
    bool stop;
};

#define sleepAnd std::this_thread::sleep_for(std::chrono::seconds(1));

int main()
{
    ThreadPool pool(12);

    std::vector<std::function<int(int)>> MaryPoppinsBag = 
    {
        [](int a) noexcept {sleepAnd return a    ;},
        [](int b) noexcept {sleepAnd return b + 1;},
        [](int c) noexcept {sleepAnd return c * 2;},
    };
    unsigned int const taskNumber = MaryPoppinsBag.size();

    std::vector<std::future<int>> results(taskNumber);
    for(unsigned int i = 0u; i < taskNumber; ++i)
        results[i] = pool.addTaskSeeFuture(MaryPoppinsBag[i], 1);


    std::cout << "Results: " << std::endl;
    for(unsigned int i = 0u; i < taskNumber; ++i)
    {
        std::cout << results[i].get(); 
        if(i == taskNumber - 1u) 
           std::cout << "";
        else
           std::cout << ", ";
    }
    std::cout << std::endl;
}
