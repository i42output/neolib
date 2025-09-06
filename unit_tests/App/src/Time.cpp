#include <neolib/app/time.hpp>

namespace
{
    void test_assert(bool assertion)
    {
        if (!assertion)
            throw std::logic_error("Test failed");
    }
}

void TestTime()
{
    auto str = neolib::to_iso8601(std::chrono::utc_clock::now());
    auto tp = neolib::from_iso8601(str);
    auto str2 = neolib::to_iso8601(tp);
    test_assert(str == str2);
}
