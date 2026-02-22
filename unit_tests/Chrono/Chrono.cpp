#include <iostream>
#include <neolib/chrono/fast_clock.hpp>

int main()
{
    std::chrono::steady_clock::now();
    std::chrono::high_resolution_clock::now();
    neolib::chrono::fast_clock::now();

    std::uint64_t n = 0;

#ifdef NDEBUG
    int constexpr iterations = 100000000;
#else
    int constexpr iterations = 10000;
#endif

    auto scStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        n += std::chrono::steady_clock::now().time_since_epoch().count();
    auto scEnd = std::chrono::high_resolution_clock::now();

    auto hrcStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        n += std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto hrcEnd = std::chrono::high_resolution_clock::now();

    auto fcStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        n += neolib::chrono::fast_clock::now().time_since_epoch().count();;
    auto fcEnd = std::chrono::high_resolution_clock::now();

    std::cout << n << std::endl;
    std::cout << "steady_clock: " << std::chrono::duration_cast<std::chrono::microseconds>(scEnd - scStart).count() << std::endl;
    std::cout << "high_resolution_clock: " << std::chrono::duration_cast<std::chrono::microseconds>(hrcEnd - hrcStart).count() << std::endl;
    std::cout << "fast_clock: " << std::chrono::duration_cast<std::chrono::microseconds>(fcEnd - fcStart).count() << std::endl;
}