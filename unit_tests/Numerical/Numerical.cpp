#include <iostream>

#include <neolib/core/numerical.hpp>
#include <neolib/core/easing.hpp>

template class neolib::basic_vector<double, 4u>;
template class neolib::basic_matrix<double, 4u, 4u>;

int main()
{
    std::cout << "ease:-" << std::endl;
    for (double t = 0.0; t <= 1.0; t += 0.05)
        std::cout << t << ": " << neolib::math::ease(neolib::math::easing::InvertedInOutQuint, t) << std::endl;
    std::cout << "partitioned_ease:-" << std::endl;
    for (double t = 0.0; t <= 1.0; t += 0.05)
        std::cout << t << ": " << neolib::math::partitioned_ease(neolib::math::easing::InvertedInOutQuint, t) << std::endl;
}