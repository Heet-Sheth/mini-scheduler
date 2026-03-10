#include<iostream>
#include<utility>
#include<thread>
using namespace std;

void f1(){
    for(int i=0;i<5;i++)
        cout<<"Thread f1."<<i<<endl;
}

void f2(){
    for(int i=0;i<5;i++)
        cout<<"Thread f2."<<i<<endl;
}

class c1{
    public:
        void fc1(){
            for(int i=0;i<5;i++)
                cout<<"Thread fc1."<<i<<endl;
        }
};

class c2{
    public:
        void fc2(){
            for(int i=0;i<5;i++)
                cout<<"Thread fc2."<<i<<endl;
        }
};

int main(){
    c1 o1;
    c2 o2;

    thread t1,t2(f1),t3(f2),t4(std::move(t3)),t5(&c1::fc1,&o1),t6(&c2::fc2,&o2);

        //t1.join();
        t2.join();
        //t3.join();
        t4.join();
        t5.join();
        t6.join();

    cout<<"all complete"<<endl;

    return 0;
}