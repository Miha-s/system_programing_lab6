#include <thread>
#include <mutex>
#include <memory>
#include <iostream>
#include <atomic>


void no_protection(long int& value) 
{
    for(int i = 0; i < 1E9; i++)
        value = value + 1;
    
    return;
}

std::mutex the_mutex;
void with_mutex(long int& value)
{
    for (size_t i = 0; i < 1E9; i++)
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
    for (size_t i = 0; i < 1E9; i++)
    {
        do
        {
            oldvalue = value.load();
            newvalue = oldvalue + 1;
        }
        while (!value.compare_exchange_strong(oldvalue, newvalue));
    }
}

int main()
{
    long int value = 0;
    std::atomic<long> atomic_value;
    atomic_value.exchange(0);

    auto f = with_mutex;

    auto func = [&value, &f]() {
        f(value);
    };
    std::thread other_thread{func};

    f(value);
    other_thread.join();

    std::cout << "Num: " << value << std::endl;

}