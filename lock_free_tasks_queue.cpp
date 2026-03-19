#include <iostream>
#include <utility>
#include <thread>
using namespace std;

class worker
{
    atomic<int> top, bottom;
    int capacity;
    deque<int> tasks;

public:
    worker(int top, int bottom, int capacity)
    {
        this->bottom = bottom;
        this->top = top;
        this->capacity = capacity;

        tasks.resize(capacity);
    }
    void push_bottom(int x)
    {
        int b = bottom.load(), t = top.load();
        if (bottom - top > capacity)
        {
            cout << "Tasks queue full" << endl;
            return;
        }
        tasks[b] = x;
        atomic_thread_fence(memory_order_release);
        bottom.store(b + 1);
    }
    int pop_bottom()
    {
        int b = bottom.load() - 1;
        bottom.store(b);
        atomic_thread_fence(memory_order_seq_cst);

        int t = top.load();

        if (t <= b)
        {
            int task = tasks[b];

            if (t == b)
            {
                if (!top.compare_exchange_strong(t, t + 1))
                    task = -1;

                bottom.store(b + 1);
            }

            return task;
        }
        else
        {
            bottom.store(t);
            return -1;
        }
    }
    int steal()
    {
        int t = top.load();
        atomic_thread_fence(memory_order_seq_cst);
        int b = bottom.load();

        if (t < b)
        {
            int task = tasks[t];

            if (top.compare_exchange_strong(t, t + 1))
                return task;
        }

        return -1;
    }
};

int main()
{
    worker w(0, 0, 5);

    for (int i = 1; i <= 10; i++)
        w.push_bottom(i);

    while (true)
    {
        int x = w.pop_bottom();

        if (x == -1)
            break;

        cout << x << ',';
    }

    return 0;
}