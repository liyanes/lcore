#include <lcore/object/obejct.hpp>

using namespace LCORE_NAMESPACE_NAME;

class A: public Object {
public:
    static TypeId GetTypeId(){
        static TypeInfo type{
            .name = "A",
            .parent = Object::GetTypeId(),
            .constructor = []{return MakePtr<A>();}
        };
        return &type;
    }
    int a = 1;
};

class B: public A {
public:
    static TypeId GetTypeId(){
        static TypeInfo type{
            .name = "B",
            .parent = A::GetTypeId(),
            .constructor = []{return MakePtr<B>();}
        };
        return &type;
    }
    int b = 2;
};

class C: public Object {
public:
    static TypeId GetTypeId(){
        static TypeInfo type{
            .name = "C",
            .parent = Object::GetTypeId(),
            .constructor = []{return MakePtr<C>();}
        };
        return &type;
    }
    int c = 3;
};

int main(){
    Ptr<Object> obj = NewObject<A>();
    obj->Aggregate(NewObject<B>());
    auto ptr = obj->GetObject<C>();
    if (ptr){
        std::cout << "Pointer not empty!" << std::endl;
        return 1;
    }
    auto ptrB = obj->GetObject<B>();
    if (!ptrB){
        std::cout << "Pointer empty" << std::endl;
        return 1;
    }
    auto ptrA = ptrB->GetObject<A>();
    if (!ptrA){
        std::cout << "Pointer empty" << std::endl;
        return 1;
    }
    auto ptrBRelease = ptrA->ReleaseObject<B>();
    if (!ptrBRelease){
        std::cout << "Pointer empty" << std::endl;
        return 1;
    }
    if (ptrA->GetObject<B>()){
        std::cout << "Pointer not empty!" << std::endl;
        return 1;
    }
    return 0;
}


