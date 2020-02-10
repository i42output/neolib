#include <iostream>
#include <neolib/tree.hpp>

int main()
{
    neolib::tree<std::string> tree;
    auto entities = tree.insert(tree.send(), "Entity");
    auto components = tree.insert(tree.send(), "Component");
    auto systems = tree.insert(tree.send(), "System");
    auto animals = tree.insert(entities.end(), "Animals");
    auto people = tree.insert(entities.end(), "People");
    auto athletes = tree.insert(people.end(), "Athletes");
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
    std::cout << "Just Animals:-" << std::endl;
    for (auto const& a : animals)
        std::cout << a << std::endl;
}
