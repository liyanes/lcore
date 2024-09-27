#include <async/executor.hpp>
#include <iostream>

using namespace LCORE_NAMESPACE_NAME;

Task<void> task1(){
    co_await std::suspend_always();
    std::cout << "Task 1 once" << std::endl;
    co_await std::suspend_always();
    std::cout << "Task 1 twice" << std::endl;
    co_await std::suspend_always();
    std::cout << "Task 1 thrice" << std::endl;
}

Task<void> task2(){
    co_await std::suspend_always();
    std::cout << "Task 2 once" << std::endl;
    co_await std::suspend_always();
    std::cout << "Task 2 twice" << std::endl;
    co_await std::suspend_always();
    std::cout << "Task 2 thrice" << std::endl;
}

int main(){
    DefaultExecutor executor;
    executor.Schedule(task1());
    executor.Schedule(task2());
    executor.Run();
    return 0;
}
