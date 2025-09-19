#include <neolib/neolib.hpp>
#include <iostream>
#include <neolib/io/string_packet.hpp>

namespace
{
    void test_assert(bool assertion)
    {
        if (!assertion)
            throw std::logic_error("Test failed");
    }
}

int main(int argc, char** argv)
{
    std::string testString = "AAAA\r\nBBBB\r\n";
    const char* first = testString.data();
    const char* last = testString.data() + testString.size();
    const char* iter = first;

    neolib::string_packet sp;

    test_assert(sp.take_some(iter, last));
    test_assert(iter == first + 6);
    test_assert(sp.length() == 4);
    test_assert(sp.contents() == "AAAA");

    sp.clear();
    test_assert(sp.take_some(iter, last));
    test_assert(iter == first + 12);
    test_assert(sp.length() == 4);
    test_assert(sp.contents() == "BBBB");

    sp.clear();
    iter = first;
    test_assert(!sp.take_some(iter, first + 4));
    test_assert(iter == first + 4);
    test_assert(sp.take_some(iter, last));
    test_assert(iter == first + 6);
    test_assert(sp.length() == 4);
    test_assert(sp.contents() == "AAAA");
    sp.clear();
    test_assert(sp.take_some(iter, last));
    test_assert(iter == first + 12);
    test_assert(sp.length() == 4);
    test_assert(sp.contents() == "BBBB");
}

