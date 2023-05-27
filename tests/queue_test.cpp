#include "TaskQueue.h"

#include <atomic>
#include <iostream>
#include <thread>

using std::clog;
using std::endl;

using namespace std::chrono_literals;

struct TestType
{
    TestType()
    {
        ++n_create;
    }

    ~TestType()
    {
        ++n_destroy;
    }

    static std::atomic<unsigned> n_create;
    static std::atomic<unsigned> n_destroy;
};

std::atomic<unsigned> TestType::n_create{0};
std::atomic<unsigned> TestType::n_destroy{0};

TaskQueue<TestType *> queue{10};
int stop_flag = 0;

#define NUM_TASK 50000

void thread_body()
{
    int n_consume = 0;
    while (n_consume < NUM_TASK)
    {
        TestType *task = nullptr;

        if (queue.fetch(task))
        {
            n_consume += 1;
            delete task;
        }
        else
        {
            std::this_thread::sleep_for(1ms);
        }
    }
}

int main()
{
    std::thread thread(thread_body);

    for (int i = 0; i < NUM_TASK; i += 1)
    {
        auto *obj = new TestType;
        while (!queue.add(obj))
            std::this_thread::sleep_for(1ms);
    }

    thread.join();
    clog << TestType::n_create << " created, " << TestType::n_destroy << " destroyed" << endl;
}
