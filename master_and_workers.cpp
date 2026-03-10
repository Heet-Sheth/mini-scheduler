#include <iostream>
#include <utility>
#include <thread>
#include <functional>
using namespace std;

queue<function<void()>> q;
mutex m;
condition_variable cv;
bool done = false;

void worker()
{
    while (true)
    {
        function<void()> ptr;

        {
            unique_lock<mutex> lock(m);
            cv.wait(lock, []
                    { return !q.empty() || done; });

            if (done && q.empty())
                break;

            ptr = q.front();
            q.pop();
        }

        ptr();
        cout << "task complete" << endl;
    }
}

void f1()
{
    cout << " executing" << endl;
}

void enqueue(int n)
{
    function<void()> ptr = f1;
    {
        cout << "task " << n << " started" << endl;
        lock_guard<mutex> lock(m);
        q.push(ptr);
    }
    cv.notify_one();
}

int main()
{
    thread t[] = {thread(worker), thread(worker), thread(worker), thread(worker), thread(worker)};

    {
        for (int i = 1; i <= 3; i++)
        {
            enqueue(i);
        }
    }

    {
        lock_guard<mutex> lock(m);
        done = true;
    }

    cv.notify_all();

    for (auto &thread : t)
    {
        thread.join();
    }

    cout << "all tasks complete" << endl;
    return 0;
}