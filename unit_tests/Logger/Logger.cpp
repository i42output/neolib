#include <thread>
#include <iostream>
#include <neolib/app/ostream_logger.hpp>

namespace neolog = neolib::logger;

void output_log_messages(neolog::i_logger& logger)
{
    for (int i = 0; i < 1000; ++i)
    {
        logger << neolog::severity::Info << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of info severity message" << neolog::endl;
        logger << neolog::severity::Debug << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of debug severity message" << neolog::endl;
        logger << neolog::severity::Debug << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of debug severity message (2 of 2)" << neolog::endl;
        logger << neolog::severity::Info << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] This is a test of info severity message via abstract interface" << neolog::endl;
    }
}

int main()
{
    try
    {
        neolog::ostream_logger logger{ std::cout };
        logger.set_filter_severity(neolog::severity::Debug);
        logger.create_logging_thread();

        std::thread thread1{ [&]()
        {
            output_log_messages(logger);
        } };

        std::thread thread2{ [&]()
        {
            output_log_messages(logger);
        } };

        std::thread thread3{ [&]()
        {
            output_log_messages(logger);
        } };

        output_log_messages(logger);

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