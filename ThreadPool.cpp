#include <ppl.h>
#include <thread>
#include <vector>

#include <concurrent_queue.h>

struct Work
{
    void run()
    {
        func();
    }
    std::function<void()> func;
};

struct ThreadPool
{
    void init(int N)
    {
        stopped = false;
        for (int i = 0; i < N; ++i)
        {
            workers.push_back(
                std::thread([this] () {
                        while (!stopped || !wqueue.empty())
                        {
                            Work w;
                            if (wqueue.try_pop(w))
                            {
                                w.run();
                            }
                        }
                    }
                )
            );
        }
    }

    void start(std::function<void()> func)
    {
        Work w;
        w.func = func;
        wqueue.push(w);
    }

    void stop()
    {
        stopped = true;
        for (auto&& t: workers)
            t.join();
    }

    concurrency::concurrent_queue<Work> wqueue;
    std::vector<std::thread>            workers;
    std::atomic<bool>                   stopped;
};

int main()
{
    ThreadPool pool;
    pool.init(10);

    for (int i = 0; i < 50; ++i)
        pool.start([]() { std::cout <<std::this_thread::get_id()<<std::endl;});

    pool.stop();
}

