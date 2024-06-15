#include <iostream>
#include <string>
#include <neolib/core/optional.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/core/jar.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/string_view.hpp>
#include <neolib/core/pair.hpp>
#include <neolib/core/segmented_array.hpp>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/polymorphic_vecarray.hpp>
#include <neolib/core/gap_vector.hpp>

struct i_foo
{
    typedef i_foo abstract_type;
};

struct foo : i_foo
{
    int n;
    foo() {};
    foo(i_foo const&) {}
};

template class neolib::basic_jar<foo>;
template class neolib::basic_std_vector_jar<foo>;

template class neolib::vecarray<int, 64, neolib::MaxSize>;
template class neolib::polymorphic::vecarray<int, 64, neolib::MaxSize>;
template class neolib::gap_vector<int>;

void TestTree();

namespace
{
    void test_assert(bool assertion)
    {
        if (!assertion)
            throw std::logic_error("Test failed");
    }
    void test_assert(int iteration, bool assertion)
    {
        if (!assertion)
        {
            std::cerr << "Test failed, iteration = " << iteration << std::endl;
            throw std::logic_error("Test failed");
        }
    }
}

int main()
{
    neolib::string_view sv{ "hello" };
    test_assert(sv == neolib::string_view{ "hello" });
    test_assert(sv == std::string{ "hello" });
    test_assert(sv == "hello");

    neolib::vecarray<int, 64, neolib::MaxSize> va;
    va.push_back(42);
    auto i = va.begin();

    neolib::gap_vector<int> gapVector;
    std::vector<int> normalVector;

    auto i1 = gapVector.begin();
    auto i2 = gapVector.begin();
    test_assert(i1 == i2);
    auto ci1 = gapVector.cbegin();
    auto ci2 = gapVector.cbegin();
    test_assert(ci1 == ci2);
    decltype(gapVector)::const_iterator ci3 = i1;
    test_assert(ci1 == i1);

    int const initCount = 20000000;

    for (int i = 1; i <= initCount; ++i)
    {
        gapVector.push_back(i);
        normalVector.push_back(i);
        test_assert(gapVector.back() == normalVector.back());
        if (i == gapVector.DefaultGapSize)
        {
            gapVector.clear();
            normalVector.clear();
        }
    }

    auto const gapVectorSize = gapVector.size();
    auto const normalVectorSize = normalVector.size();
    test_assert(gapVectorSize == normalVectorSize);
    test_assert(std::distance(gapVector.begin(), gapVector.end()) == std::distance(normalVector.begin(), normalVector.end()));
    test_assert(std::equal(gapVector.begin(), gapVector.end(), normalVector.begin()));

    std::chrono::high_resolution_clock::time_point gapStart;
    std::chrono::high_resolution_clock::time_point gapEnd;

    int const walkCount = 10000;

    {
        gapStart = std::chrono::high_resolution_clock::now();
        srand(0);
        int index = gapVector.size() / 2;
        for (int i = 1; i <= walkCount; ++i)
        { 
            index = index + rand() % gapVector.DefaultGapSize - gapVector.DefaultGapSize / 2;
            index = std::max<int>(0, std::min<int>(index, gapVector.size() - 1));
            auto const n2 = std::min<std::size_t>(gapVector.size() - index, 4);
            auto const size = gapVector.size();
            switch (rand() % 4)
            {
            case 0:
                gapVector.insert(std::next(gapVector.begin(), index), rand());
                test_assert(i, gapVector.size() == size + 1);
                break;
            case 1:
                gapVector.insert(std::next(gapVector.begin(), index), { 1, 2, 3, 4 });
                test_assert(i, gapVector.size() == size + 4);
                break;
            case 2:
                gapVector.erase(std::next(gapVector.begin(), index));
                test_assert(i, gapVector.size() == size - 1);
                break;
            case 3:
                {
                    auto const first = std::next(gapVector.begin(), index);
                    auto const last = std::next(gapVector.begin(), index + n2);
                    gapVector.erase(first, last);
                    test_assert(i, gapVector.size() == size - n2);
                }
                break;
            }
        }
        gapEnd = std::chrono::high_resolution_clock::now();
    }

    std::chrono::high_resolution_clock::time_point normalStart;
    std::chrono::high_resolution_clock::time_point normalEnd;

    {
        normalStart = std::chrono::high_resolution_clock::now();
        srand(0);
        int index = normalVector.size() / 2;
        for (int i = 1; i <= walkCount; ++i)
        {
            index = index + rand() % gapVector.DefaultGapSize - gapVector.DefaultGapSize / 2;
            index = std::max<int>(0, std::min<int>(index, normalVector.size() - 1));
            auto const n2 = std::min<std::size_t>(normalVector.size() - index, 4);
            auto const size = normalVector.size();
            switch (rand() % 4)
            {
            case 0:
                normalVector.insert(std::next(normalVector.begin(), index), rand());
                test_assert(i, normalVector.size() == size + 1);
                break;
            case 1:
                normalVector.insert(std::next(normalVector.begin(), index), { 1, 2, 3, 4 });
                test_assert(i, normalVector.size() == size + 4);
                break;
            case 2:
                normalVector.erase(std::next(normalVector.begin(), index));
                test_assert(i, normalVector.size() == size - 1);
                break;
            case 3:
                {
                    auto const first = std::next(normalVector.begin(), index);
                    auto const last = std::next(normalVector.begin(), index + n2);
                    normalVector.erase(first, last);
                    test_assert(i, normalVector.size() == size - n2);
                }
                break;
            }
        }
        normalEnd = std::chrono::high_resolution_clock::now();
    }

    auto const gapVectorSize2 = gapVector.size();
    auto const normalVectorSize2 = normalVector.size();
    test_assert(gapVectorSize2 == normalVectorSize2);
    test_assert(std::equal(gapVector.begin(), gapVector.end(), normalVector.begin()));
    test_assert(std::equal(normalVector.begin(), normalVector.end(), gapVector.begin()));
    test_assert(std::equal(gapVector.rbegin(), gapVector.rend(), normalVector.rbegin()));
    test_assert(std::equal(normalVector.rbegin(), normalVector.rend(), gapVector.rbegin()));

    decltype(gapVector) gapVectorReversed;
    std::copy(gapVector.rbegin(), gapVector.rend(), std::back_inserter(gapVectorReversed));
    std::reverse(gapVectorReversed.begin(), gapVectorReversed.end());
    test_assert(gapVector == gapVectorReversed);

    // todo: more gap_vector unit tests, e.g. multi-element insert/erase.

    std::cout << "neolib::gap_vector: " << std::chrono::duration_cast<std::chrono::milliseconds>(gapEnd - gapStart).count() / 1000.0 << " s" << std::endl;
    std::cout << "std::vector: " << std::chrono::duration_cast<std::chrono::milliseconds>(normalEnd - normalStart).count() / 1000.0 << " s" << std::endl;

    neolib::string s1, s2;
    neolib::i_string const& rs1{ s1 };
    neolib::i_string const& rs2{ s2 };

    test_assert(s1 == s2);
    test_assert(s1 == rs2);
    test_assert(rs2 == s1);

    neolib::optional<neolib::string> os1;
    neolib::i_optional<neolib::i_string>& raos1{ os1 };
    test_assert(os1 == os1);
    test_assert(os1 == raos1);
    test_assert(raos1 == os1);

    neolib::pair<neolib::string, neolib::string> p1;
    neolib::pair<neolib::string, neolib::string> p2;

    test_assert(p1 == p2);
    test_assert(!(p1 < p2));
    test_assert(!(p1 > p2));

    neolib::variant<neolib::string, int, double> v;
    neolib::variant<neolib::string, int, double, foo> v2;
    neolib::variant<neolib::string, int, double, foo> v3{ neolib::string{} };
    neolib::variant<neolib::string, int, double, foo> v4{ std::string{} };
    neolib::variant<neolib::string, int, double, foo> v5{ v4 };
    neolib::variant<neolib::string, int, double, foo> v6{ static_cast<neolib::abstract_t<decltype(v4)> const&>(v4) };

    using bv = neolib::variant<neolib::string, int, double, foo>;

    struct dv : bv
    {
        using bv::bv;
        using bv::operator=;

        dv(const bv& other) : bv{ other } {}
        dv(bv&& other) : bv{ std::move(other) } {}
    };

    dv dv1;
    dv dv2{ dv1 };
    dv dv3{ v2 };

    static_assert(!decltype(v)::is_alternative_v<std::string>);
    static_assert(decltype(v)::is_alternative_v<neolib::string>);
    static_assert(decltype(v)::is_alternative_v<neolib::i_string>);
    static_assert(decltype(v)::is_alternative_v<const neolib::string&>);
    static_assert(decltype(v)::is_alternative_v<const neolib::i_string&>);

    test_assert(v == neolib::none);
    test_assert(!(v != neolib::none));

    v = neolib::string{};
    v = std::string{};
    
    v <=> v;

    v2 = neolib::none;

    test_assert(!(v < v));
    test_assert(v == v);
    test_assert(!(v != v));

    neolib::optional<foo> of = {};

    neolib::optional<bool> o1 = true;
    neolib::optional<bool> o2 = neolib::optional<bool>{ true };
    neolib::optional<bool> o3 = false;
    neolib::optional<bool> o4 = neolib::optional<bool>{ false };

    std::optional<bool> so1{ o1.to_std_optional() };
    std::optional<bool> so2{ o2.to_std_optional() };
    std::optional<bool> so3{ o3.to_std_optional() };
    std::optional<bool> so4{ o4.to_std_optional() };

    test_assert(*o1 == true);
    test_assert(*o2 == true);
    test_assert(*o3 == false);
    test_assert(*o4 == false);

    test_assert(*so1 == true);
    test_assert(*so2 == true);
    test_assert(*so3 == false);
    test_assert(*so4 == false);

    neolib::basic_jar<foo> jar;
    jar.emplace();
    jar.emplace();
    jar.emplace();

    jar.item_cookie(jar.at_index(1));

    neolib::segmented_array<int> sa;
    sa.push_back(1);
    sa.push_back(2);
    sa.push_back(3);
    
    ++sa.begin();
    ++sa.cbegin();
    sa.begin()++;
    sa.cbegin()++;
    --++sa.begin();
    --++sa.cbegin();
    (++sa.begin())--;
    (++sa.cbegin())--;

    TestTree();
}

