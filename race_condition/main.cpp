#include <thread>
#include <mutex>
#include <memory>
#include <iostream>
#include <atomic>
#include <unistd.h>

long count = 1E8;

void no_protection(long int& value) 
{
    for(int i = 0; i < count; i++)
        value = value + 1;
    
    return;
}

std::mutex the_mutex;
void with_mutex(long int& value)
{
    for (size_t i = 0; i < count; i++)
    {
        {
            std::lock_guard guard{the_mutex};
            value = value+1;
        }
    }
    
}

void with_atomic(std::atomic<long>& value)
{
    long oldvalue, newvalue;
    for (size_t i = 0; i < count; i++)
    {
        do
        {
            oldvalue = value.load();
            newvalue = oldvalue + 1;
        }
        while (!value.compare_exchange_strong(oldvalue, newvalue));
    }
}

void sync_increase(long int& counter)
{
    for(int i = 0; i < 1000; i++) {
        int current = counter;
        std::this_thread::yield();
        counter = current + 1;
        std::this_thread::yield();
    }
}

int main()
{
    long int value = 0;
    std::atomic<long> atomic_value;
    atomic_value.exchange(0);
    std::cout << "pid: " << getpid() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // auto atomic_f = [&atomic_value](long value){
    //     with_atomic(atomic_value);
    // };
    // auto f = no_protection; 

    // auto func = [&value, &f]() {
    //     f(value);
    // };
    // std::thread other_thread{func};

    // f(value);
    // other_thread.join();

    // std::cout << "Num: " << value << std::endl;

    value = 0;
    std::thread sync_thread{[&value](){
        sync_increase(value);
    }};
    std::thread sync_thread2{[&value](){
        sync_increase(value);
    }};

    sync_thread2.join();
    sync_thread.join();

    std::cout << "Value: " << value << std::endl;
}