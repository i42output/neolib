#include <vector>
#include <thread>
#include <iostream>
#include <neolib/app/i_shared_thread_local.hpp>

int next_sequence()
{
    static std::atomic<int> sequence = 0;
    return ++sequence;
}

struct wibble
{
    wibble() { std::cout << "wibble::wibble(), thread id: " << std::this_thread::get_id() << std::endl; }
    ~wibble() { std::cout << "wibble::~wibble(), thread id: " << std::this_thread::get_id() << std::endl; }
};

template <typename T>
struct wobble
{
    wobble() { f(); }
    int& f() { shared_thread_local_class(int, *this, f, n, next_sequence()); return n; }
    static int& sf() { shared_thread_local_class(int, wobble<T>, sf, n, next_sequence()); return n; }
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

namespace
{
    void test_assert(bool assertion)
    {
        if (!assertion)
            throw std::logic_error("Test failed");
    }
}

int main()
{
    neolib::allocate_service_provider();

    auto test = []()
    {
        test_assert(foo::f() == foo::f());
        test_assert(bar::f() == bar::f());
        test_assert(foo::f() != bar::f());

        std::cout << foo::f() << " " << bar::f() << std::endl;

        wobble<int> o1;
        wobble<double> o2;

        test_assert(&o1.f() == &o1.f());
        test_assert(&o2.f() == &o2.f());
        test_assert(&o1.f() != &o2.f());
        test_assert(&o1.sf() == &o1.sf());
        test_assert(&o2.sf() == &o2.sf());
        test_assert(&o1.sf() != &o2.sf());

        test_assert(o1.f() == o1.f());
        test_assert(o2.f() == o2.f());
        test_assert(o1.f() != o2.f());
        test_assert(o1.sf() == o1.sf());
        test_assert(o2.sf() == o2.sf());
        test_assert(o1.sf() != o2.sf());
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