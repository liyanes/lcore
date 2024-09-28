#include <lcore/object/obejct.hpp>

using namespace LCORE_NAMESPACE_NAME;

class A: public Object {
public:
    int a = 1;
};

class B: public A {
public:
    int b = 2;
};

class C: public Object {
public:
    int c = 3;
};

int main(){
    Ptr<Object> obj = MakePtr<A>();
    obj->Aggregate(MakePtr<B>());
    auto ptr = obj->GetObject<C>();
    if (ptr){
        std::cout << "Pointer not empty!" << std::endl;
    }
    auto ptrB = obj->GetObject<B>();
    if (!ptrB){
        std::cout << "Pointer empty" << std::endl;
    }
    return 0;
}


