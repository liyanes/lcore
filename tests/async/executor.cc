#include <gtest/gtest.h>
#include <lcore/async/executor.hpp>
#include <iostream>
#include <sstream>

using namespace LCORE_NAMESPACE_NAME::async;

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

TEST(ExecutorTest, BasicFunctionality) {
    std::stringstream output;
    std::streambuf* originalBuffer = std::cout.rdbuf(output.rdbuf());
    
    DefaultExecutor<> executor;
    executor.Schedule(task1());
    executor.Schedule(task2());
    executor.Run();

    std::cout.rdbuf(originalBuffer); // Restore original buffer

    std::string expectedOutput = "Task 1 once\nTask 2 once\nTask 1 twice\nTask 2 twice\nTask 1 thrice\nTask 2 thrice\n";
    EXPECT_EQ(output.str(), expectedOutput);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

