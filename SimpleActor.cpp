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

#include <iostream>
#include <string>
#include <map>

using namespace std;

namespace Act
{
    struct System
    {
        System()
        {
            pool.init(10);
        }

        void send(string const& name, string const& msg)
        {
            if (actors.find(name) == actors.end())
            {
                cerr << "No actor " << name << endl;
                return;
            }

            pool.start([this, msg, name](){actors[name](msg); });
        }

        void stop(string const& name)
        {
            actors.erase(name);
        }

        typedef function<void(string const&)>   Actor;

        void start(string const& name, Actor act)
        {
            actors.insert(make_pair(name, act));
        }

        map<string, Actor>  actors;
        ThreadPool          pool;
    };

    System system;

    void pingActor(string const& msg)
    {
        static int cnt = 0;
        if (msg == "ping")
        {
            cnt++;
            cout << msg << " " << std::this_thread::get_id()<< endl;
            if (cnt == 5)
            {
                system.send("pingActor", "stop");
                system.send("pongActor", "stop");
            }
            else
                system.send("pongActor", "pong");
        }
        else if (msg == "stop")
        {
            system.stop("pingActor");
        }
        else
        {
            cerr << "Wrong message: " << msg << endl;
        }
    }

    void pongActor(string const& msg)
    {
        if (msg == "pong")
        {
            cout << msg << " " <<std::this_thread::get_id() << endl;
            system.send("pingActor", "ping");
        }
        else if (msg == "stop")
        {
            system.stop("pongActor");
        }
        else
        {
            cerr << "Wrong message: " << msg << endl;
        }
    }
}

struct ReachPing
{
    int cnt = 0;
    void operator()(string const& msg)
    {
        if (msg == "ping")
        {
            cnt++;
            cout << msg << " " << std::this_thread::get_id() << endl;
            if (cnt == 5)
            {
                Act::system.send("pingActor", "stop");
                Act::system.send("pongActor", "stop");
            }
            else
                Act::system.send("pongActor", "pong");
        }
        else if (msg == "stop")
        {
            Act::system.stop("pingActor");
        }
        else
        {
            cerr << "Wrong message: " << msg << endl;
        }
    }
};

int main()
{
    Act::system.start("pingActor", ReachPing());
    Act::system.start("pongActor", Act::pongActor);
    //Act::system.send("pingActor", "asdasd");
    Act::system.send("pingActor", "ping");
    //Act::system.send("pingActor", "stop");

    Act::system.pool.stop();
}
