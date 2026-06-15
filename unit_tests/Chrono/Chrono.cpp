#include <iostream>
#include <neolib/chrono/fast_clock.hpp>

int main()
{
    std::chrono::steady_clock::now();
    std::chrono::high_resolution_clock::now();
    neolib::chrono::fast_clock::now();

    std::uint64_t defeatOptimiser = 0;

#ifdef NDEBUG
    int constexpr iterations = 100000000;
#else
    int constexpr iterations = 10000;
#endif

    auto scStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        defeatOptimiser += std::chrono::steady_clock::now().time_since_epoch().count();
    auto scEnd = std::chrono::high_resolution_clock::now();

    auto hrcStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        defeatOptimiser += std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto hrcEnd = std::chrono::high_resolution_clock::now();

    auto fcStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        defeatOptimiser += neolib::chrono::fast_clock::now().time_since_epoch().count();
    auto fcEnd = std::chrono::high_resolution_clock::now();

    std::cout << "steady_clock(1): " << std::chrono::duration_cast<std::chrono::microseconds>(scEnd - scStart).count() << std::endl;
    std::cout << "high_resolution_clock(1): " << std::chrono::duration_cast<std::chrono::microseconds>(hrcEnd - hrcStart).count() << std::endl;
    std::cout << "fast_clock(1): " << std::chrono::duration_cast<std::chrono::microseconds>(fcEnd - fcStart).count() << std::endl;

    auto t0 = std::make_pair(std::chrono::high_resolution_clock::now(), neolib::chrono::fast_clock::now());
    std::this_thread::sleep_for(std::chrono::milliseconds{ 5000 });
    auto t1 = std::make_pair(std::chrono::high_resolution_clock::now(), neolib::chrono::fast_clock::now());

    std::cout << "high_resolution_clock(2): " << std::chrono::duration_cast<std::chrono::milliseconds>(t1.first - t0.first) << std::endl;
    std::cout << "fast_clock(2): " << std::chrono::duration_cast<std::chrono::milliseconds>(t1.second - t0.second) << std::endl;

    auto t2 = std::make_pair(std::chrono::high_resolution_clock::now(), neolib::chrono::fast_clock::now());
    for (int i = 0; i < 100000; ++i)
        defeatOptimiser += std::rand();
    auto t3 = std::make_pair(std::chrono::high_resolution_clock::now(), neolib::chrono::fast_clock::now());

    std::cout << "high_resolution_clock(3): " << std::chrono::duration_cast<std::chrono::microseconds>(t3.first - t2.first) << std::endl;
    std::cout << "fast_clock(3): " << std::chrono::duration_cast<std::chrono::microseconds>(t3.second - t2.second) << std::endl;

    std::cout << defeatOptimiser << std::endl;
}