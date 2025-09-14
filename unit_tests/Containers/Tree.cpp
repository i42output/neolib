#include <iostream>
#include <string>
#include <neolib/core/tree.hpp>

void TestTree()
{
    neolib::tree<int, 2> tree0;
    for (int i = 0; i < 100; ++i)
        tree0.insert(tree0.kend(), 42);
    for (int i = 0; i < 10000; ++i)
    {
        tree0.erase(std::next(tree0.kbegin(), std::rand() % tree0.size()));
        tree0.insert(std::next(tree0.kbegin(), std::rand() % tree0.size()), 42);
    }
    while (!tree0.empty())
        tree0.erase(tree0.kbegin());

    neolib::tree<std::string> tree;
    auto entities = tree.insert(tree.send(), "Entity");
    auto components = tree.insert(tree.send(), "Component");
    auto systems = tree.insert(tree.send(), "System");
    auto shapes = tree.insert(entities.end(), "Shapes");
    auto animals = tree.insert(entities.end(), "Animals");
    auto people = tree.insert(entities.end(), "People");
    auto athletes = tree.insert(people.end(), "Athletes (London 2012 Gold Medalists, Running)");
    tree.push_back(shapes, "Square");
    tree.push_back(shapes, "Triangle");
    tree.push_back(shapes, "Circle");
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

    tree.sort();
    std::cout << std::endl;
    std::cout << "Entire tree (sorted, with descendent counts):-" << std::endl;
    for (auto i = tree.begin(); i != tree.end(); ++i)
        std::cout << std::string(i.depth() * 4, ' ') << *i << " (" << i.descendent_count() << ")" << std::endl;

    // note: saved iterators are invalidated after the sort.

    components = std::find(tree.sbegin(), tree.send(), "Component");
    tree.erase(components);
    std::cout << std::endl;
    std::cout << "Tree after leaf node erase:-" << std::endl;
    for (auto i = tree.begin(); i != tree.end(); ++i)
        std::cout << std::string(i.depth() * 4, ' ') << *i << " (" << i.descendent_count() << ")" << std::endl;

    shapes = std::find(tree.begin(), tree.end(), "Shapes");
    tree.erase(shapes);
    std::cout << std::endl;
    std::cout << "Tree after branch node erase:-" << std::endl;
    for (auto i = tree.begin(); i != tree.end(); ++i)
        std::cout << std::string(i.depth() * 4, ' ') << *i << " (" << i.descendent_count() << ")" << std::endl;

    neolib::tree<std::string> tree2;
    auto connections = tree2.insert(tree2.send(), "Connections");
    auto identity = tree2.insert(connections.end(), "Identity");
    auto network = tree2.insert(identity.end(), "Network");
    auto console = tree2.insert(network.end(), "Console");
    auto channel = tree2.insert(network.end(), "Channel");

    std::cout << std::endl;
    std::cout << "Entire tree2 (with depth):-" << std::endl;
    for (auto i = tree2.begin(); i != tree2.end(); ++i)
        std::cout << std::string(i.depth() * 4, ' ') << *i << std::endl;

    tree2.erase(console);
    tree2.erase(network);

    std::cout << std::endl;
    std::cout << "Entire tree2 after erase (with depth):-" << std::endl;
    for (auto i = tree2.begin(); i != tree2.end(); ++i)
        std::cout << std::string(i.depth() * 4, ' ') << *i << std::endl;

    /*
    std::cout << std::endl;
    std::cout << "Just Animals:-" << std::endl;
    for (auto const& a : animals)
        std::cout << a << std::endl; */
}
