#include <iostream>
#include <string>
#include <neolib/core/optional.hpp>
#include <neolib/core/tree.hpp>
#include <neolib/core/jar.hpp>

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

template class neolib::segmented_tree<std::string, 64, std::allocator<std::string>>;

int main()
{
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

    neolib::tree<std::string> tree;
    auto entities = tree.insert(tree.send(), "Entity");
    auto components = tree.insert(tree.send(), "Component");
    auto systems = tree.insert(tree.send(), "System");
    auto animals = tree.insert(entities.end(), "Animals");
    auto people = tree.insert(entities.end(), "People");
    auto athletes = tree.insert(people.end(), "Athletes (London 2012 Gold Medalists, Running)");
    tree.push_back(animals, "Dolphin");
    tree.push_back(animals, "Kitten");
    tree.push_back(animals, "Hedgehog");
    tree.push_back(athletes, "Usain Bolt");
    tree.push_back(athletes, "Usain Bolt");
    tree.push_back(athletes, "Kirani James");
    tree.push_back(athletes, "David Rudisha");
    tree.push_back(athletes, "Taoufik Makhloufi");
    tree.push_back(athletes, "Mo Farah");
    tree.push_back(athletes, "Mo Farah");
    tree.push_back(athletes, "Shelly-Ann Fraser-Pryce");
    tree.push_back(athletes, "Allyson Felix");
    tree.push_back(athletes, "Sanya Richards-Ross");
    tree.push_back(athletes, "Caster Semenya");
    tree.push_back(athletes, "Maryam Yusuf Jamal");
    tree.push_back(athletes, "Meseret Defar Tola");
    tree.push_back(athletes, "Tirunesh Dibaba Kenene");

    std::cout << "Entire tree:-" << std::endl;
    for (auto const& e : tree)
        std::cout << e << std::endl;

    std::cout << std::endl;
    std::cout << "Entire tree (with depth):-" << std::endl;
    for (auto i = tree.begin(); i != tree.end(); ++i)
        std::cout << std::string(i.depth() * 4, ' ') << *i << std::endl;

    std::cout << std::endl;
    std::cout << "Entire tree (reverse iteration, with depth):-" << std::endl;
    for (auto i = tree.rbegin(); i != tree.rend(); ++i)
        std::cout << std::string(std::prev(i.base()).depth() * 4, ' ') << *i << std::endl;

    std::cout << std::endl;
    std::cout << "Entire tree (sorted, with descendent counts):-" << std::endl;
    tree.sort();
    for (auto i = tree.begin(); i != tree.end(); ++i)
        std::cout << std::string(i.depth() * 4, ' ') << *i << " (" << i.descendent_count() << ")" << std::endl;
    /*
    std::cout << std::endl;
    std::cout << "Just Animals:-" << std::endl;
    for (auto const& a : animals)
        std::cout << a << std::endl; */
}
