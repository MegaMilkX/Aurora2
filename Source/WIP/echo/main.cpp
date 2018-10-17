#include "echo.hpp"
#include "variant.hpp"

#include <iostream>
#include <string>

class MyType {
public:
    int counter;

    void foo() {

    }

    void bar(int a, float b) {

    }

    std::string hello(int q) {
        return "hello";
    }
};

int main() {
    echo::reg::type<MyType>("MyType")
        .property("counter", &MyType::counter)
        .method("foo", &MyType::foo)
        .method("bar", &MyType::bar)
        .method("hello", &MyType::hello)
        .meta("editor_visible", false);
    echo::reg::type<std::string>("string");


    echo::type t = echo::type::get<MyType>();
    echo::type t2 = echo::type::get("MyType");

    std::cout << echo::type::get<int>().get_name() << std::endl;
    std::cout << echo::type::get<std::string>().get_name() << std::endl;
    std::cout << echo::type::get<MyType>().get_name() << std::endl;

    std::cout << echo::type::get<MyType>().get_name() << " has " << echo::type::get<MyType>().get_properties().size() << " properties" << std::endl;

    echo::property prop = echo::type::get<MyType>().get_property("counter");

    echo::variant var = 10.0f;
    if(var.is<float>()) {
        std::cout << "var is float" << std::endl;
    } else {
        std::cout << "var is not float" << std::endl;
    }

    std::getchar();

    return 0;
}