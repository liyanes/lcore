#include <async/utils.hpp>
#include <traits.hpp>
#include <vector>
#include <list>
#include <iostream>
#include <tuple>

using namespace lcore;

int main(){
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int> lst = {6, 7, 8, 9, 10};
    std::vector<double> dvec = {1.1, 2.2, 3.3, 4.4, 5.5};

    auto pd = product(vec, lst, dvec);
    for(auto [v, l, d]: pd){
        std::cout << v << " " << l << " " << d << std::endl;
    }
}
