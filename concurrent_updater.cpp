#include<iostream>
#include<utility>
#include<thread>
using namespace std;

atomic_int counter;

void f1(){
    for(int i=0;i<1000;i++)
        counter++;
}

int main(){

    thread t1(f1),t2(f1),t3(f1);

    t1.join();
    t2.join();
    t3.join();

    cout<<"final values counter:"<<counter<<endl;

    return 0;
}