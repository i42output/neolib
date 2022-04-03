#include <iostream>
#include <string>
#include <neolib/core/optional.hpp>
#include <neolib/core/jar.hpp>
#include <neolib/core/string_utils.hpp>

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

int main()
{
    std::vector<std::string> v0, v1, v2, v3, v4, v5;
    neolib::tokens(""s, ","s, v0, 0, false);
    neolib::tokens("1"s, ","s, v1, 0, false);
    neolib::tokens("1,"s, ","s, v2, 0, false);
    neolib::tokens("1,2"s, ","s, v3, 0, false);
    neolib::tokens(",2"s, ","s, v4, 0, false);
    neolib::tokens(","s, ","s, v5, 0, false);

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

    TestTree();
}

