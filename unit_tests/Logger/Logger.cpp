#include <thread>
#include <iostream>
#include <neolib/app/ostream_logger.hpp>

namespace neolog = neolib::logger;

enum class category : int32_t
{
    Red,
    Green,
    Blue,
    Black,
    White
};

const neolog::category Red{ category::Red };
const neolog::category Green{ category::Green };
const neolog::category Blue{ category::Blue };
const neolog::category Black{ category::Black };
const neolog::category White{ category::White };

void output_log_messages(neolog::i_logger& logger0, neolog::i_logger& logger1)
{
    for (int i = 0; i < 1000; ++i)
    {
        logger0 << Red << neolog::severity::Info << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] (Red) Info message 1" << neolog::endl;
        logger0 << Green << neolog::severity::Debug << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] (Green) Debug message 1" << neolog::endl;
        logger0 << Blue << neolog::severity::Debug << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] (Blue) Debug message 2" << neolog::endl;
        logger0 << Black << neolog::severity::Info << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] (Black) Info message 2" << neolog::endl;
        logger0 << White << neolog::severity::Info << "[tid: " << std::this_thread::get_id() << "] [" << std::hex << "0x" << i << "] (White) Info message 3" << neolog::endl;

        logger1 << neolog::severity::Info << "LOGGER1 MESSAGE" << neolog::flush;
    }
}

int main()
{
    try
    {
        neolog::ostream_logger<0> logger0{ std::cout };
        logger0.set_filter_severity(neolog::severity::Debug);
        logger0.create_logging_thread();

        neolog::ostream_logger<1> logger1{ std::cerr };
        logger1.create_logging_thread();

        neolog::formatter logger1Formmatter{ [](neolog::i_logger const& /* aLogger */, neolib::i_string const& aUnformattedMessage, neolib::i_string& aFormattedMessage)
        {
            thread_local std::string temp;
            temp = "OoOo " + aUnformattedMessage.to_std_string() + " oOoO\n";
            aFormattedMessage = temp;
        } };
        logger1.set_formatter(logger1Formmatter);

        /* std::ofstream ofs{ "c:\\tmp\\test.log" };
        neolog::ostream_logger<2> logger2{ ofs };
        logger2.create_logging_thread();
        logger0.copy_to(logger2); */

        logger0.register_category(category::Red);
        logger0.register_category(category::Green);
        logger0.register_category(category::Blue);
        logger0.register_category(category::Black);
        logger0.register_category(category::White);

        logger0.disable_category(category::White);
        // logger2.enable_category(category::White);

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