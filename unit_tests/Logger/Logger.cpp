#include <thread>
#include <iostream>
#include <neolib/app/ostream_logger.hpp>

namespace neolog = neolib::logger;

void output_log_messages(neolog::i_logger& logger0, neolog::i_logger& logger1)
{
    for (int i = 0; i < 1000; ++i)
    {
        logger0 << neolog::severity::Info << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of info severity message" << neolog::endl;
        logger0 << neolog::severity::Debug << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of debug severity message" << neolog::endl;
        logger0 << neolog::severity::Debug << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of debug severity message (2 of 2)" << neolog::endl;
        logger0 << neolog::severity::Info << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of info severity message via abstract interface" << neolog::endl;

        logger1 << neolog::severity::Info << "**** LOGGER1 MESSAGE ****" << neolog::endl;
    }
}

int main()
{
    try
    {
        neolog::ostream_logger<0> logger0{ std::cout };
        logger0.set_filter_severity(neolog::severity::Debug);
        logger0.create_logging_thread();

        neolog::ostream_logger<1> logger1{ std::cout };
        logger1.create_logging_thread();

        std::thread thread1{ [&]()
        {
            output_log_messages(logger0, logger1);
        } };

        std::thread thread2{ [&]()
        {
            output_log_messages(logger0, logger1);
        } };

        std::thread thread3{ [&]()
        {
            output_log_messages(logger0, logger1);
        } };

        output_log_messages(logger0, logger1);

        thread1.join();
        thread2.join();
        thread3.join();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown exception" << std::endl;
    }
}