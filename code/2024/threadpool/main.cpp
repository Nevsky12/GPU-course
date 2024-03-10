#include <iostream>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono> 
#include <random>
#include <stop_token>
#include <string>
#include <cstdio>

std::queue<int> taskQueue;
std::vector<std::jthread> threadPool;
std::mutex mtx;
// Tp - thread pool, P - producer, C - consumer
std::condition_variable communicatorTpPC;

int main()
{
    int const timeC = 0;
    auto const consumer = [timeC](std::stop_source ss) noexcept
    {
        std::printf("Worker %ld started\n", pthread_self());
        std::stop_token tok = ss.get_token();
        while(!tok.stop_requested())
        {
            std::unique_lock ul{mtx};
            
            communicatorTpPC.wait
            (
                ul, 
                [&tok]{return !taskQueue.empty() || tok.stop_requested();}
            );
            
            if(tok.stop_requested())
            {
                std::printf("Worker %ld is requested to stop\n", pthread_self());
                ul.unlock();
                break;
            }

            int const task = taskQueue.front();
            std::printf("Worker %ld work with task %d\n", pthread_self(), task);
            taskQueue.pop();
            ul.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(timeC));
            std::printf("Worker %ld goes back to sleep\n", pthread_self());
        }
    };

    int const size = 7;
    threadPool.resize(size);
    std::stop_source s = std::stop_source();
    for(unsigned int i = 0; i < size; ++i)
    {
        threadPool[i] = std::jthread(consumer, s);
    }

    int const timeP     = 1;
    int const taskCount = 20;
    int i = 0;
    while(true)
    {
        ++i;

        {
            std::lock_guard lg{mtx};
            taskQueue.push(i);
            communicatorTpPC.notify_one(); 
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(timeP));

        if(i > taskCount)
        {
            communicatorTpPC.notify_all();
            s.request_stop();
            break;
        }
    }

    return 0;
}
