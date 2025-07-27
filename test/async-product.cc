#include <lcore/async/utils.hpp>
#include <lcore/traits.hpp>
#include <vector>
#include <list>
#include <iostream>
#include <tuple>

using namespace LCORE_NAMESPACE_NAME::async;

int main(){
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int> lst = {6, 7, 8, 9, 10};
    std::vector<double> dvec = {1.1, 2.2, 3.3, 4.4, 5.5};

    auto pd = product(vec, lst, dvec);
    for(auto [v, l, d]: pd){
        std::cout << v << " " << l << " " << d << std::endl;
    }
}
