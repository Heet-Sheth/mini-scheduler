#include <iostream>
#include <thread>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;

struct Worker
{
    deque<int> q;
    mutex m;
    int id;

    Worker(int id_) : id(id_) {}
};

vector<Worker *> workers;
atomic<bool> done{false};

void worker_thread(Worker *self)
{
    std::mt19937 rng(std::random_device{}());

    while (true)
    {
        int task = -1;

        // 1️⃣ Try local work
        {
            lock_guard<mutex> lock(self->m);
            if (!self->q.empty())
            {
                task = self->q.front();
                self->q.pop_front();
            }
        }

        // 2️⃣ If no local work → try stealing
        if (task == -1)
        {

            bool stole = false;

            for (int attempt = 0; attempt < workers.size(); ++attempt)
            {
                int victim_index = rng() % workers.size();
                Worker *victim = workers[victim_index];

                if (victim == self)
                    continue;

                lock_guard<mutex> lock(victim->m);

                if (!victim->q.empty())
                {
                    task = victim->q.back(); // steal from back
                    victim->q.pop_back();
                    cout << "Worker " << self->id
                         << " stole from Worker "
                         << victim->id << endl;
                    stole = true;
                    break;
                }
            }

            if (!stole)
            {
                break; // no work anywhere → exit
            }
        }

        // 3️⃣ Execute task
        if (task != -1)
        {
            this_thread::sleep_for(chrono::milliseconds(task));
            cout << "Worker " << self->id
                 << " executed task " << task << endl;
        }
    }
}

int main()
{

    int num_workers = 6;

    // Create workers
    for (int i = 0; i < num_workers; ++i)
    {
        workers.push_back(new Worker(i));
    }

    // Force imbalance:
    // Only worker 0 has tasks
    for (int i = 1; i <= 20; ++i)
    {
        workers[0]->q.push_back(50); // equal duration tasks
    }

    vector<thread> threads;

    for (int i = 0; i < num_workers; ++i)
    {
        threads.emplace_back(worker_thread, workers[i]);
    }

    for (auto &t : threads)
        t.join();

    cout << "All tasks completed." << endl;
    return 0;
}