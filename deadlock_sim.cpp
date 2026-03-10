#include <iostream>
#include <utility>
#include <thread>
using namespace std;

atomic_int pc{0};
mutex a, b;

void worker1()
{
    cout << "t1 trying to lock A" << endl;
    a.lock();
    this_thread::sleep_for(1000ms);
    cout << "t1 trying to lock B" << endl;
    b.lock();

    pc++;

    a.unlock();
    b.unlock();
}
void worker2()
{
    cout << "t2 trying to lock B" << endl;
    b.lock();
    this_thread::sleep_for(1000ms);
    cout << "t2 trying to lock A" << endl;
    a.lock();

    pc++;

    a.unlock();
    b.unlock();
}
void watchDog()
{
    static int pastValue = 0;

    this_thread::sleep_for(500ms);

    if (pc > pastValue)
        pastValue = pc;
    else
        cout << "Deadlock !!!";
}

int main()
{
    thread t1(worker1), t2(worker2), t3(watchDog);

    t1.join();
    t2.join();
    t3.join();

    cout << "Able to complete" << endl;
    return 0;
}