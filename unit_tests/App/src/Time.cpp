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
    auto const now = std::chrono::utc_clock::now();
    auto str = neolib::to_iso8601(now);
    auto tp = neolib::from_iso8601(str);
    auto str2 = neolib::to_iso8601(tp);
    test_assert(str == str2);
    auto str3 = neolib::to_iso8601(now, false);
    auto tp2 = neolib::from_iso8601(str3);
    auto str4 = neolib::to_iso8601(tp2, false);
    test_assert(str3 == str4);
}
