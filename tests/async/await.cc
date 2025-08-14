#include <gtest/gtest.h>
#include <lcore/async/task.hpp>
#include <lcore/async/awaiter.hpp>
#include <lcore/async/executor.hpp>

using namespace lcore;
using namespace lcore::async;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class AsyncAwaitTest : public ::testing::Test {
protected:
    DefaultExecutor<> executor;
    
    void SetUp() override {
        // Initialize the executor or any other setup needed for tests
    }
    void TearDown() override {
        // Clean up resources if necessary
    }
};

/// =================== 1 ===================

void func(std::function<void(int)> callback) {
    callback(42);
}

Task<void> asyncFunc() {
    auto ret = co_await MakeCallbackAwaiter(func);
    EXPECT_EQ(ret, 42);
    co_return;
}


class ExampleClass {
public:
    ExampleClass() { 
        std::cout << "ExampleClass constructor called" << std::endl; 
    }
    ExampleClass(const ExampleClass&) = delete; // Disable copy constructor
    ExampleClass(ExampleClass&&) noexcept {}
    ~ExampleClass() { 
        std::cout << "ExampleClass destructor called" << std::endl; 
    }
};

void funcWithClass(std::function<void(ExampleClass)> callback) {
    callback(ExampleClass());
}

Task<void> asyncFuncWithClass() {
    auto ret = co_await MakeCallbackAwaiter(funcWithClass);
    // Here we can check if the ExampleClass was constructed and destructed properly
    // This is just a placeholder, as we can't directly check the construction/destruction in this context
    co_return;
}

TEST_F(AsyncAwaitTest, BasicAwaiterTest) {
    executor.Schedule(asyncFunc());
    executor.Run();

    std::stringstream output;
    std::streambuf* oldCoutBuffer = std::cout.rdbuf(output.rdbuf());
    executor.Schedule(asyncFuncWithClass());
    executor.Run();
    std::cout.rdbuf(oldCoutBuffer);

    // Check if the ExampleClass constructor and destructor messages were printed
    auto str = output.str();
    EXPECT_NE(str.find("ExampleClass constructor called"), std::string::npos);
    EXPECT_NE(str.find("ExampleClass destructor called"), std::string::npos);
    EXPECT_EQ(str.find("ExampleClass constructor called"), str.rfind("ExampleClass constructor called"));
}

/// =================== 2 ===================

