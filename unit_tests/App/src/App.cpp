#include <vector>
#include <thread>
#include <iostream>
#include <neolib/app/i_shared_thread_local.hpp>

int next_sequence()
{
    static std::atomic<int> sequence;
    return ++sequence;
}

struct wibble
{
    wibble() { std::cout << "wibble::wibble(), thread id: " << std::this_thread::get_id() << std::endl; }
    ~wibble() { std::cout << "wibble::~wibble(), thread id: " << std::this_thread::get_id() << std::endl; }
};

namespace foo
{
    int f()
    {
        shared_thread_local(int, foo::f, n, next_sequence());
        shared_thread_local(wibble, foo::f, o);
        shared_thread_local(std::vector<int>, foo::f, v, {});
        v.push_back(42);
        return n;
    }
}

namespace bar
{
    int f()
    {
        shared_thread_local(int, bar::f, n, next_sequence());
        shared_thread_local(wibble, bar::f, o);
        shared_thread_local(std::vector<int>, bar::f, v, {});
        v.push_back(42);
        return n;
    }
}

int main()
{
    neolib::allocate_service_provider();

    auto test = []()
    {
        assert(foo::f() == foo::f());
        assert(bar::f() == bar::f());
        assert(foo::f() != bar::f());
        std::cout << foo::f() << " " << bar::f() << std::endl;
    };
    test();
    std::thread t1{ test };
    std::thread t2{ test };
    std::thread t3{ test };
    std::thread t4{ test };
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}