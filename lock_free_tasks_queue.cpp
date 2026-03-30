#include <iostream>
#include <utility>
#include <thread>
#include <random>
#include <limits>

using namespace std;

class worker
{
    atomic<int> top, bottom;
    int capacity;
    vector<int> tasks;

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

        if (b - t >= capacity)
        {
            cout << "Tasks queue full" << endl;
            return;
        }
        tasks[b % capacity] = x;
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
            int task = tasks[b % capacity];

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
            int task = tasks[t % capacity];

            if (top.compare_exchange_strong(t, t + 1))
                return task;
        }

        return -1;
    }
};

atomic<int> numW;
vector<worker *> workers;

void push_bottom(worker *w)
{
    for (int i = 1; i <= 10; i++)
        w->push_bottom(i);
}
void pop_bottom(worker *w)
{

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, numW - 1);

    while (true)
    {
        int x = w->pop_bottom();
        if (x == -1)
        {
            for (int i = 0; i < numW; i++)
            {
                int victim = dist(gen);

                if (workers[victim] == w)
                    continue;

                x = workers[victim]->steal();

                if (x == -1)
                    break;

                cout << x << endl;
            }
        }
        else
            cout << x << ',';
    }
}

int main()
{
    numW = 10;

    workers.push_back(new worker(0, 0, 5));

    for (int i = 1; i < numW; i++)
        workers.push_back(new worker(0, 0, 5));

    vector<thread *> threads;

    threads.push_back(new thread(push_bottom, workers[0]));

    for (int i = 0; i < numW; i++)
        threads.push_back(new thread(pop_bottom, workers[i]));

    for (int i = 0; i < numW; i++)
        threads[i]->join();

    cout << "Completed!!";

    return 0;
}