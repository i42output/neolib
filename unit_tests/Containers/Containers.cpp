#include <iostream>
#include <string>
#include <neolib/core/optional.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/core/jar.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/pair.hpp>
#include <neolib/core/segmented_array.hpp>
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

template class neolib::gap_vector<int>;

void TestTree();

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
    neolib::gap_vector<int> gapVector;
    std::vector<int> normalVector;

    for (int i = 1; i < 1000000; ++i)
    {
        gapVector.push_back(i);
        normalVector.push_back(i);
    }

    test_assert(std::equal(gapVector.begin(), gapVector.end(), normalVector.begin()));

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

