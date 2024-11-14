#include <lcore/async/task.hpp>
#include <lcore/async/generator.hpp>
#include <lcore/async/agenerator.hpp>
#include <iostream>
#include <ranges>
#include <memory>

using namespace LCORE_NAMESPACE_NAME;

Generator<int> yiledtest(){
    co_yield 1;
    co_yield 2;
    co_yield 3;
}

Generator<std::shared_ptr<int>> yiledtest2(){
    co_yield std::make_shared<int>(1);
    co_yield std::make_shared<int>(2);
    co_yield std::make_shared<int>(3);
}

Generator<int> viewtest(){
    int i;
    while (true){
        co_yield i++;
    }
}

Task<int> tasktest(){
    co_return 1;
};

Task<int> taskawaittest(){
    co_await tasktest();
    co_return 2;
};

AsyncGenerator<int> agen(){
    co_await std::suspend_always();
    co_yield 1;
    co_yield 2;
    co_yield 3;
}

Task<void> voidtext(){
    co_await std::suspend_always();
    std::cout << "Void test" << std::endl;
}

int main(){
    std::cout << "For yiledtest:" << std::endl;
    for (auto i: yiledtest()){
        std::cout << i << std::endl;
    }
    std::cout << "For yiledtest2:" << std::endl;
    for (auto i: yiledtest2()){
        std::cout << *i << std::endl;
    }
    std::cout << "For viewtest:" << std::endl;
    for (auto i: viewtest() | std::views::take(10)){
        std::cout << i << std::endl;
    }
    std::cout << "For tasktest:" << std::endl;
    std::cout << tasktest().get() << std::endl;

    std::cout << "For taskawaittest:" << std::endl;
    auto taskawait = taskawaittest();
    while (!taskawait.done()) taskawait.resume();
    std::cout << taskawait.get() << std::endl;

    std::cout << "For agen:" << std::endl;
    for (auto i: agen()){
        while (!i.done()) i.resume();
        // In async generator, we need to manually check if the generator will yield a value
        if (!i.has_value()) break;
        std::cout << i.get() << std::endl;
    }

    std::cout << "For voidtext:" << std::endl;
    voidtext().resume();
};
