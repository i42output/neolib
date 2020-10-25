#include <iostream>
#include <neolib/app/ostream_logger.hpp>

namespace neolog = neolib::logger;

int main()
{
    neolog::ostream_logger logger{ std::cout };
    logger << neolog::severity::Info << "This is a test of info severity message" << neolog::endl;
    logger << neolog::severity::Debug << "This is a test of debug severity message" << neolog::endl;
    logger.set_filter(neolog::severity::Debug);
    logger << neolog::severity::Debug << "This is a test of debug severity message (2 of 2)" << neolog::endl;

    neolog::i_logger& abstractLogger = logger;
    abstractLogger << neolog::severity::Info << "This is a test of info severity message via abstract interface" << neolog::endl;
}