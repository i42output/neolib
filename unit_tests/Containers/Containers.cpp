#include <iostream>
#include <string>
#include <neolib/core/optional.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/core/jar.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/pair.hpp>
#include <neolib/core/segmented_array.hpp>

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

void TestTree();

#undef NDEBUG

int main()
{
    neolib::string s1, s2;
    neolib::i_string const& rs1{ s1 };
    neolib::i_string const& rs2{ s2 };

    assert(s1 == s2);
    assert(s1 == rs2);
    assert(rs2 == s1);

    neolib::optional<neolib::string> os1;
    neolib::i_optional<neolib::i_string>& raos1{ os1 };
    assert(os1 == os1);
    assert(os1 == raos1);
    assert(raos1 == os1);

    neolib::pair<neolib::string, neolib::string> p1;
    neolib::pair<neolib::string, neolib::string> p2;

    assert(p1 == p2);
    assert(!(p1 < p2));
    assert(!(p1 > p2));

    neolib::variant<neolib::string, int, double> v;
    neolib::variant<neolib::string, int, double, foo> v2;

    assert(v == neolib::none);
    assert(!(v != neolib::none));
    
    v <=> v;

    v2 = neolib::none;

    assert(!(v < v));
    assert(v == v);
    assert(!(v != v));

    neolib::optional<foo> of = {};

    neolib::optional<bool> o1 = true;
    neolib::optional<bool> o2 = neolib::optional<bool>{ true };
    neolib::optional<bool> o3 = false;
    neolib::optional<bool> o4 = neolib::optional<bool>{ false };

    std::optional<bool> so1{ o1.to_std_optional() };
    std::optional<bool> so2{ o2.to_std_optional() };
    std::optional<bool> so3{ o3.to_std_optional() };
    std::optional<bool> so4{ o4.to_std_optional() };

    assert(*o1 == true);
    assert(*o2 == true);
    assert(*o3 == false);
    assert(*o4 == false);

    assert(*so1 == true);
    assert(*so2 == true);
    assert(*so3 == false);
    assert(*so4 == false);

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

