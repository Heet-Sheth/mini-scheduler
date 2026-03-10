#include <iostream>
#include <utility>
#include <thread>
using namespace std;

mutex m;
condition_variable not_empty;
condition_variable not_full;
const size_t capacity = 5;
queue<int> q;
bool done = false;

void producer(int N)
{
    for (int i = 0; i < N; i++)
    {
        {
            unique_lock<mutex> lock(m);
            not_full.wait(lock, []
                          { return q.size() < capacity; });
            q.push(i);
        }
        not_empty.notify_one();
        cout << "item added" << endl;
        this_thread::sleep_for(5ms);
    }

    {
        lock_guard<mutex> lock(m);
        done = true;
    }
    not_empty.notify_all();
}

void consumer()
{
    while (true)
    {
        int x;
        {
            unique_lock<mutex> lock(m);
            not_empty.wait(lock, []
                           { return !q.empty() || done; });

            if (done && q.empty())
                break;
            else
            {
                x = q.front();
                q.pop();
            }
        }

        not_full.notify_one();

        cout << "item consumed" << x << endl;
    }
}

int main()
{
    int n = 20;

    thread p1(producer, n);
    thread c1(consumer);

    p1.join();
    c1.join();

    cout << "Work done!!!" << endl;

    return 0;
}