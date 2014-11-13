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
                std::thread([this]() {
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
        for (auto&& t : workers)
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
};

atomic<bool> fork[5] = { 0 };

#include <mutex>

std::mutex cout_mutex;

#define SYNC_OUT(code) \
{\
std::lock_guard<std::mutex> _(cout_mutex);\
code\
}\

class Philosopher
{
    //think
    //hungry
    //eating

    //enum State{Think, Hungry, Eating};
    //static string states[3] = { "Think", "Hungry", "Eating" };

    string state = "Think";
    int pos;
    string name;

public:

    Philosopher(int p, string const& n)
        :pos(p), name(n)
    {
    }

    string getName() const
    {
        return name;
    }

    void operator()(string const& msg)
    {
        if (state == "Think")
            think(msg);
        else if (state == "Hungry")
            hungry(msg);
        else
            eating(msg);
    }

    void think(string const& msg)
    {
        if (msg == "TakeFork")
        {
            cerr << "Cannot take fork while thinking!" << endl;
            return;
        }

        if (msg == "eat")
        {
            cerr << "Cannot start eat while thinking!" << endl;
            return;
        }

        if (msg == "hungry")
        {
            state = "Hungry";
            SYNC_OUT(cout << name << " become hungry" << endl;)
            Act::system.send(name, "TakeFork");
            Act::system.send(name, "TakeFork");
        }
    }

    void hungry(string const& msg)
    {
        if (msg == "hungry")
        {
            cerr << "Cannot become hungry while hungry!" << endl;
            return;
        }

        if (msg == "eat")
        {
            cerr << "Cannot start eat while hungry!" << endl;
            return;
        }

        static bool leftFork = false;
        static bool rightFork = false;

        const int rightIdx = pos;
        const int leftIdx = (pos + 1) % 5;

        if (msg == "TakeFork")
        {
            if (!rightFork)
            {
                if (fork[rightIdx])
                    Act::system.send(name, "TakeFork");
                else
                {
                    fork[rightIdx] = true;
                    rightFork = true;
                    SYNC_OUT(cout << name << " take right fork" << endl;)
                }                   
            }
            else
            {
                if (fork[leftIdx])
                    Act::system.send(name, "TakeFork");
                else
                {
                    fork[leftIdx] = true;
                    leftFork = true;
                    leftFork = rightFork = false;

                    SYNC_OUT(cout << name << " take left fork" << endl;)

                    state = "Eating";
                    Act::system.send(name, "eat");
                }
            }
        }
    }

    void eating(string const& msg)
    {
        if (msg == "hungry")
        {
            cerr << "Cannot start eat while eating!" << endl;
            return;
        }

        if (msg == "TakeFork")
        {
            cerr << "Cannot take fork while eating!" << endl;
            return;
        }

        if (msg == "eat")
        {
            SYNC_OUT(cout << name << " start eating" << endl;)
            std::this_thread::sleep_for(chrono::seconds(3));
            SYNC_OUT(cout << name << " stop eating" << endl;)
            state = "Think";
            fork[pos] = false;
            fork[pos + 1] = false;
        }
    }
};


int main()
{
    vector<Philosopher> phils;
    for (int i = 0; i < 5; ++i)
    {
        phils.push_back(Philosopher(i, "Phil #" + to_string(i)));
        fork[i] = false;
    }

    for (auto& p : phils)
        Act::system.start(p.getName(), p);

    for (auto& p : phils)
        Act::system.send(p.getName(), "hungry");

    Act::system.pool.stop();
}

