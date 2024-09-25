#include <container/circulelist.hpp>
#include <iostream>

using namespace lcore;

int main(){
    CircularList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    auto iter = list.ignore_begin();
    auto num = 6;
    while (num--){
        std::cout << *iter << std::endl;
        ++iter;
    }

    list.erase(list.begin());
    num = 4;
    iter = list.ignore_begin();
    while (num--){
        std::cout << *iter << std::endl;
        ++iter;
    }
}
