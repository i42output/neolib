#include <neolib/neolib.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <neolib/core/service.hpp>
#include <neolib/task/async_task.hpp>
#include <neolib/task/event.hpp>
#include <neolib/io/binary_packet.hpp>
#include <neolib/io/string_packet.hpp>
#include <neolib/io/packet_stream.hpp>
#include <neolib/io/tcp_packet_stream_server.hpp>

namespace
{
    using namespace std::chrono_literals;

    std::uint32_t sFailures = 0u;

    void test_assert(bool aAssertion, const std::string& aWhat = "assertion")
    {
        if (!aAssertion)
            throw std::logic_error("Test failed: " + aWhat);
    }

    std::string describe(const boost::system::error_code& aError)
    {
        return aError.category().name() + std::string{ ":" } + std::to_string(aError.value()) +
            " (" + aError.message() + ")";
    }

    void run_test(const char* aName, const std::function<void()>& aTest)
    {
        std::cout << "  " << aName << " ... " << std::flush;
        try
        {
            aTest();
            std::cout << "ok" << std::endl;
        }
        catch (const std::exception& e)
        {
            ++sFailures;
            std::cout << "FAILED (" << e.what() << ")" << std::endl;
        }
        catch (...)
        {
            ++sFailures;
            std::cout << "FAILED (unknown exception)" << std::endl;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // string_packet::take_some
    ///////////////////////////////////////////////////////////////////////////

    void test_string_packet_crlf_lines()
    {
        std::string testString = "AAAA\r\nBBBB\r\n";
        const char* first = testString.data();
        const char* last = testString.data() + testString.size();
        const char* iter = first;

        neolib::string_packet sp;

        test_assert(sp.take_some(iter, last), "first line not terminated");
        test_assert(iter == first + 6, "first line did not consume CRLF");
        test_assert(sp.length() == 4, "first line length");
        test_assert(sp.contents() == "AAAA", "first line contents");

        sp.clear();
        test_assert(sp.take_some(iter, last), "second line not terminated");
        test_assert(iter == first + 12, "second line did not consume CRLF");
        test_assert(sp.length() == 4, "second line length");
        test_assert(sp.contents() == "BBBB", "second line contents");

        test_assert(!sp.take_some(iter, last), "exhausted range must return false");
    }

    void test_string_packet_split_across_reads()
    {
        // the same input, but delivered in two chunks with the split falling
        // inside the first line's payload
        std::string testString = "AAAA\r\nBBBB\r\n";
        const char* first = testString.data();
        const char* last = testString.data() + testString.size();
        const char* iter = first;

        neolib::string_packet sp;

        test_assert(!sp.take_some(iter, first + 4), "partial line must not terminate");
        test_assert(iter == first + 4, "partial line must consume what it saw");
        test_assert(sp.contents() == "AAAA", "partial line must accumulate");

        test_assert(sp.take_some(iter, last), "line should terminate once CRLF arrives");
        test_assert(iter == first + 6, "terminating call must stop after CRLF");
        test_assert(sp.contents() == "AAAA", "accumulated contents must not be duplicated");

        sp.clear();
        test_assert(sp.take_some(iter, last), "second line not terminated");
        test_assert(sp.contents() == "BBBB", "second line contents");
    }

    void test_string_packet_empty_range()
    {
        neolib::string_packet sp;
        const char* iter = nullptr;
        test_assert(!sp.take_some(iter, iter), "empty range must return false");
        test_assert(sp.empty(), "empty range must not produce contents");
    }

    void test_string_packet_leading_delimiters()
    {
        // a read starting on a delimiter must be skipped without fabricating
        // contents; the connection relies on this to drop empty packets
        std::string testString = "\r\n\r\nAAAA\r\n";
        const char* iter = testString.data();
        const char* last = testString.data() + testString.size();

        neolib::string_packet sp;

        test_assert(sp.take_some(iter, last), "terminating delimiter reported");
        test_assert(sp.empty(), "delimiter-only take must yield an empty packet");

        test_assert(sp.take_some(iter, last), "second terminating delimiter reported");
        test_assert(sp.empty(), "delimiter-only take must yield an empty packet");

        test_assert(sp.take_some(iter, last), "payload line not terminated");
        test_assert(sp.contents() == "AAAA", "payload contents");
    }

    void test_string_packet_lf_only_line_endings()
    {
        std::string testString = "AAAA\nBBBB\n";
        const char* iter = testString.data();
        const char* last = testString.data() + testString.size();

        neolib::string_packet sp;

        test_assert(sp.take_some(iter, last), "LF must terminate a line");
        test_assert(sp.contents() == "AAAA", "LF line contents");
        sp.clear();
        test_assert(sp.take_some(iter, last), "LF must terminate a line");
        test_assert(sp.contents() == "BBBB", "LF line contents");
    }

    void test_string_packet_trailing_cr()
    {
        // pins current behaviour: a CR at the end of a read chunk is taken as a
        // line terminator, and the LF that follows in the next chunk is
        // consumed as a leading delimiter yielding an empty packet
        std::string chunk1 = "AAAA\r";
        std::string chunk2 = "\nBBBB\r\n";

        neolib::string_packet sp;

        const char* iter = chunk1.data();
        test_assert(sp.take_some(iter, chunk1.data() + chunk1.size()), "trailing CR terminates");
        test_assert(sp.contents() == "AAAA", "trailing CR contents");
        sp.clear();

        iter = chunk2.data();
        const char* last = chunk2.data() + chunk2.size();
        test_assert(sp.take_some(iter, last), "orphaned LF reported as terminated");
        test_assert(sp.empty(), "orphaned LF must not fabricate contents");
        test_assert(sp.take_some(iter, last), "following line not terminated");
        test_assert(sp.contents() == "BBBB", "following line contents");
    }

    void test_string_packet_accumulates_long_line()
    {
        std::string const payload(5000u, 'x');
        std::string const wire = payload + "\r\n";

        neolib::string_packet sp;
        std::size_t const chunkSize = 1024u;
        std::size_t offset = 0u;
        bool terminated = false;
        while (offset < wire.size())
        {
            std::size_t const thisChunk = std::min(chunkSize, wire.size() - offset);
            const char* iter = wire.data() + offset;
            const char* last = iter + thisChunk;
            while (iter != last)
                terminated = sp.take_some(iter, last) || terminated;
            offset += thisChunk;
        }

        test_assert(terminated, "long line never terminated");
        test_assert(sp.length() == payload.size(), "long line length");
        test_assert(sp.contents() == payload, "long line contents");
    }

    void test_string_packet_data_clone_copy()
    {
        neolib::string_packet empty;
        bool threw = false;
        try
        {
            (void)empty.data();
        }
        catch (const neolib::i_packet::packet_empty&)
        {
            threw = true;
        }
        test_assert(threw, "data() on an empty packet must throw packet_empty");

        neolib::string_packet source{ "PING :irc.example.org" };
        test_assert(source.length() == 21u, "source length");
        test_assert(!source.has_max_length(), "string_packet is unbounded");

        auto cloned = source.clone();
        test_assert(cloned != nullptr, "clone returned null");
        test_assert(cloned->length() == source.length(), "clone length");
        test_assert(std::string(cloned->data(), cloned->length()) == source.contents(), "clone contents");
        test_assert(cloned->data() != source.data(), "clone must not alias the source");

        neolib::string_packet target{ "stale" };
        target.copy_from(source);
        test_assert(target.contents() == source.contents(), "copy_from contents");

        target.clear();
        test_assert(target.empty(), "clear must empty the packet");

        neolib::string_packet fromRange{ source.data(), 4u };
        test_assert(fromRange.contents() == "PING", "pointer/length construction");
    }

    void test_binary_packet_take_some()
    {
        char const wire[] = { 'a', 'b', 'c', '\0', 'd' };
        neolib::binary_packet bp;

        const char* iter = &wire[0];
        const char* last = &wire[0] + sizeof(wire);

        test_assert(bp.take_some(iter, last), "binary take_some must report a packet");
        test_assert(iter == last, "binary take_some must consume the whole range");
        test_assert(bp.length() == sizeof(wire), "binary packet length");
        test_assert(bp.contents()[3] == '\0', "binary packet must be 8-bit clean");
        test_assert(!bp.take_some(iter, last), "exhausted range must return false");

        auto cloned = bp.clone();
        test_assert(cloned->length() == bp.length(), "binary clone length");
    }

    ///////////////////////////////////////////////////////////////////////////
    // loopback harness
    ///////////////////////////////////////////////////////////////////////////

    class io_fixture
    {
    public:
        io_fixture() :
            iTask{ "neolib::io unit test" }
        {
        }
        ~io_fixture()
        {
            // let any outstanding completions run while the io_context is still
            // alive, rather than leaving them to be destroyed during its shutdown
            auto const deadline = std::chrono::steady_clock::now() + 2000ms;
            while (iTask.io_context().poll(false) && std::chrono::steady_clock::now() < deadline)
                std::this_thread::yield();
        }
    public:
        neolib::async_task& task()
        {
            return iTask;
        }
        bool pump(const std::function<bool()>& aDone, std::chrono::milliseconds aTimeout = 5000ms)
        {
            auto const deadline = std::chrono::steady_clock::now() + aTimeout;
            for (;;)
            {
                if (aDone())
                    return true;
                iTask.io_context().poll(false);
                if (std::chrono::steady_clock::now() >= deadline)
                    return aDone();
                std::this_thread::yield();
            }
        }
        void settle(std::chrono::milliseconds aFor = 250ms)
        {
            auto const deadline = std::chrono::steady_clock::now() + aFor;
            while (std::chrono::steady_clock::now() < deadline)
            {
                iTask.io_context().poll(false);
                std::this_thread::yield();
            }
        }
    private:
        neolib::async_task iTask;
    };

    template <typename PacketType>
    std::string packet_to_string(const PacketType& aPacket)
    {
        return aPacket.empty() ? std::string{} : std::string(aPacket.data(), aPacket.length());
    }

    // a server that records everything it sees, and optionally echoes it back
    template <typename PacketType>
    class test_server
    {
    public:
        typedef neolib::packet_stream<PacketType, neolib::tcp_protocol> stream_type;
    public:
        test_server(neolib::async_task& aTask, unsigned short aPort, bool aEcho = false,
            std::shared_ptr<boost::asio::ssl::context> aSecureContext = nullptr) :
            iEcho{ aEcho },
            iServer{ aTask, aPort, aSecureContext != nullptr, neolib::IPv4, aSecureContext }
        {
            iSink += iServer.packet_stream_added([this](stream_type& aStream)
            {
                ++iConnections;
                iLastStream = &aStream;
                iSink += aStream.packet_arrived([this, &aStream](const PacketType& aPacket)
                {
                    iReceived.push_back(packet_to_string(aPacket));
                    if (iEcho)
                    {
                        std::string const echo = iReceived.back() + "\r\n";
                        aStream.send_packet(PacketType{ echo.data(), echo.size() });
                    }
                });
                iSink += aStream.transfer_failure([this](const boost::system::error_code& aError)
                {
                    ++iTransferFailures;
                    iLastTransferFailure = describe(aError);
                });
                // registered after the server's own slot on the same event, so
                // this only gets called if the server stops destroying the
                // stream from inside the trigger
                iSink += aStream.connection_closed([this]()
                {
                    ++iStreamConnectionClosed;
                });
            });
            // NOT aStream.connection_closed: tcp_packet_stream_server::accept_connection
            // subscribes to that event first and destroys the stream from inside
            // the trigger, so any slot added later is never reached. The server's
            // own PacketStreamRemoved is raised from the server and is safe.
            iSink += iServer.packet_stream_removed([this](stream_type& aStream)
            {
                ++iConnectionsClosed;
                // still alive at this point: the server holds it until the end
                // of this trigger
                iLastCloseHadError = aStream.has_error();
                iLastCloseError = iLastCloseHadError ?
                    std::to_string(aStream.error_code()) + " (" + aStream.error() + ")" :
                    std::string{};
                if (iLastStream == &aStream)
                    iLastStream = nullptr;
            });
        }
    public:
        const std::vector<std::string>& received() const { return iReceived; }
        std::uint32_t connections() const { return iConnections; }
        std::uint32_t connections_closed() const { return iConnectionsClosed; }
        std::uint32_t transfer_failures() const { return iTransferFailures; }
        const std::string& last_transfer_failure() const { return iLastTransferFailure; }
        bool last_close_had_error() const { return iLastCloseHadError; }
        std::uint32_t stream_connection_closed() const { return iStreamConnectionClosed; }
        const std::string& last_close_error() const { return iLastCloseError; }
        stream_type* last_stream() const { return iLastStream; }
    private:
        bool iEcho;
        neolib::tcp_packet_stream_server<PacketType> iServer;
        neolib::sink iSink;
        std::vector<std::string> iReceived;
        std::uint32_t iConnections = 0u;
        std::uint32_t iConnectionsClosed = 0u;
        std::uint32_t iTransferFailures = 0u;
        std::string iLastTransferFailure;
        bool iLastCloseHadError = false;
        std::uint32_t iStreamConnectionClosed = 0u;
        std::string iLastCloseError;
        stream_type* iLastStream = nullptr;
    };

    unsigned short find_free_port(neolib::async_task& aTask)
    {
        for (unsigned short port = 45000u; port < 45200u; ++port)
        {
            try
            {
                neolib::tcp_packet_stream_server<neolib::string_packet> probe{ aTask, port };
                return port;
            }
            catch (...)
            {
            }
        }
        throw std::runtime_error("no free loopback port in 45000-45199");
    }

    // opens the client if necessary and pumps until the connection is established
    void connect(io_fixture& aFixture, neolib::tcp_string_packet_stream& aClient, neolib::sink& aSink, unsigned short aPort)
    {
        bool connected = false;
        aSink += aClient.connection_established([&connected]() { connected = true; });
        if (!aClient.opened())
            aClient.open("127.0.0.1", aPort);
        test_assert(aFixture.pump([&connected]() { return connected; }), "client failed to connect");
    }

    ///////////////////////////////////////////////////////////////////////////
    // loopback tests
    ///////////////////////////////////////////////////////////////////////////

    void test_loopback_round_trip()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port, true };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        std::vector<std::string> echoed;
        sink += client.packet_arrived([&echoed](const neolib::string_packet& aPacket)
        {
            echoed.push_back(packet_to_string(aPacket));
        });
        connect(fix, client, sink, port);

        // the client's connect completes before the server's accept handler
        // necessarily has, so wait for it rather than sampling immediately
        test_assert(fix.pump([&server]() { return server.connections() >= 1u; }), "server did not accept the connection");
        test_assert(server.connections() == 1u, "server accepted more than one connection");

        client.send_packet(neolib::string_packet{ "NICK tester\r\n" });
        client.send_packet(neolib::string_packet{ "USER a b c :d\r\n" });

        test_assert(fix.pump([&server]() { return server.received().size() >= 2u; }), "server did not receive both packets");
        test_assert(server.received()[0] == "NICK tester", "first packet contents");
        test_assert(server.received()[1] == "USER a b c :d", "second packet contents");

        // delimiters are part of what you send and are stripped from what you
        // receive
        test_assert(fix.pump([&echoed]() { return echoed.size() >= 2u; }), "client did not receive the echo");
        test_assert(echoed[0] == "NICK tester", "first echo contents");
        test_assert(echoed[1] == "USER a b c :d", "second echo contents");

        test_assert(server.transfer_failures() == 0u, "unexpected transfer failure: " + server.last_transfer_failure());
        test_assert(!client.has_error(), "client reported an error");
    }

    void test_loopback_packets_share_one_write()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        connect(fix, client, sink, port);

        // three lines in a single send, so the receiver has to split them
        client.send_packet(neolib::string_packet{ "ONE\r\nTWO\r\nTHREE\r\n" });

        test_assert(fix.pump([&server]() { return server.received().size() >= 3u; }), "server did not split the batched lines");
        test_assert(server.received()[0] == "ONE", "batched line 1");
        test_assert(server.received()[1] == "TWO", "batched line 2");
        test_assert(server.received()[2] == "THREE", "batched line 3");

        fix.settle();
        test_assert(server.received().size() == 3u, "server produced spurious packets");
    }

    void test_loopback_line_larger_than_receive_buffer()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        connect(fix, client, sink, port);

        // the default receive buffer is 1024 bytes, so this line has to be
        // reassembled across several reads
        std::string const payload(8000u, 'z');
        client.send_packet(neolib::string_packet{ payload + "\r\n" });

        test_assert(fix.pump([&server]() { return !server.received().empty(); }, 10000ms), "oversized line never arrived");
        test_assert(server.received().size() == 1u, "oversized line was split into several packets");
        test_assert(server.received()[0] == payload, "oversized line contents");
        test_assert(server.transfer_failures() == 0u, "unexpected transfer failure: " + server.last_transfer_failure());
    }

    void test_loopback_unterminated_line_is_withheld()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        connect(fix, client, sink, port);

        client.send_packet(neolib::string_packet{ "PART" });
        fix.settle();
        test_assert(server.received().empty(), "an unterminated line must not be delivered");

        client.send_packet(neolib::string_packet{ "IAL\r\n" });
        test_assert(fix.pump([&server]() { return !server.received().empty(); }), "line never completed");
        test_assert(server.received()[0] == "PARTIAL", "reassembled line contents");
    }

    void test_loopback_clean_close_is_not_a_transfer_failure()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        std::uint32_t clientTransferFailures = 0u;
        std::string clientLastFailure;
        sink += client.transfer_failure([&](const boost::system::error_code& aError)
        {
            ++clientTransferFailures;
            clientLastFailure = describe(aError);
        });
        connect(fix, client, sink, port);

        client.close();
        test_assert(fix.pump([&server]() { return server.connections_closed() >= 1u; }), "server never saw the close");

        fix.settle();
        test_assert(server.transfer_failures() == 0u,
            "remote close reported as a transfer failure: " + server.last_transfer_failure());
        test_assert(clientTransferFailures == 0u,
            "local close reported as a transfer failure: " + clientLastFailure);
        test_assert(client.closed(), "client should be closed");
    }

    void test_connection_failure_to_dead_port()
    {
        io_fixture fix;
        // nothing is listening here: find_free_port releases the probe acceptor
        auto const port = find_free_port(fix.task());

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        bool failed = false;
        bool connected = false;
        sink += client.connection_failure([&failed](const boost::system::error_code&) { failed = true; });
        sink += client.connection_established([&connected]() { connected = true; });

        client.open("127.0.0.1", port);

        test_assert(fix.pump([&failed]() { return failed; }), "connection to a dead port did not fail");
        test_assert(!connected, "connection to a dead port must not succeed");
        test_assert(!client.connected(), "stream must not report connected");
        test_assert(client.has_error(), "stream must record the error");
    }

    void test_binary_stream_round_trip()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::binary_packet> server{ fix.task(), port };

        neolib::tcp_binary_packet_stream client{ fix.task() };
        neolib::sink sink;
        bool connected = false;
        sink += client.connection_established([&connected]() { connected = true; });
        client.open("127.0.0.1", port);
        test_assert(fix.pump([&connected]() { return connected; }), "binary client failed to connect");

        std::string const wire{ "\x01\x02\x00\x03\x7f", 5u };
        client.send_packet(neolib::binary_packet{ wire.data(), wire.size() });

        std::string assembled;
        test_assert(fix.pump([&]()
        {
            assembled.clear();
            for (auto const& r : server.received())
                assembled += r;
            return assembled.size() >= wire.size();
        }), "binary payload never arrived");
        test_assert(assembled == wire, "binary payload contents");
    }

    ///////////////////////////////////////////////////////////////////////////
    // regression: basic_packet_connection::close() must drop pending sends
    ///////////////////////////////////////////////////////////////////////////

    // basic_packet_connection's send queue holds raw pointers into the owning
    // packet_stream's queue. packet_stream::close() destroys those packets
    // (remove_all_packets) before calling connection::close(). If close() does
    // not clear the connection's own queue, the next successful connect pops a
    // dangling pointer and hands freed memory to async_write.
    void test_close_drops_queued_packets()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;

        // queued while not connected, so these sit in the connection's send
        // queue rather than going out on the wire
        client.send_packet(neolib::string_packet{ "STALE ONE\r\n" });
        client.send_packet(neolib::string_packet{ "STALE TWO\r\n" });

        client.close();

        // churn the heap so that a surviving dangling pointer reads recycled
        // memory rather than an intact copy of the freed packet
        {
            std::vector<std::string> churn;
            for (int i = 0; i != 512; ++i)
                churn.emplace_back(64u, static_cast<char>('A' + (i % 26)));
        }

        connect(fix, client, sink, port);

        client.send_packet(neolib::string_packet{ "FRESH\r\n" });

        test_assert(fix.pump([&server]() { return !server.received().empty(); }), "nothing arrived after reopening");
        fix.settle();

        test_assert(server.received().size() == 1u, "close() left stale packets in the send queue");
        test_assert(server.received()[0] == "FRESH", "unexpected packet contents after reopening");
        test_assert(server.transfer_failures() == 0u, "unexpected transfer failure after reopening: " + server.last_transfer_failure());
        test_assert(!client.has_error(), "client reported an error after reopening");
    }

    // the connection's in-flight packet pointer is only cleared by
    // handle_write. Closing while a write is outstanding orphans the handler
    // proxy, so if close() does not reset it the send_any() guard blocks every
    // subsequent send for the life of the connection object.
    void test_close_clears_packet_being_sent()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        connect(fix, client, sink, port);

        // starts the async_write immediately; close before it can complete
        client.send_packet(neolib::string_packet{ "IN FLIGHT\r\n" });
        client.close();

        test_assert(fix.pump([&server]() { return server.connections_closed() >= 1u; }), "server never saw the close");

        neolib::sink sink2;
        connect(fix, client, sink2, port);

        client.send_packet(neolib::string_packet{ "AFTER REOPEN\r\n" });

        test_assert(fix.pump([&server]()
        {
            for (auto const& r : server.received())
                if (r == "AFTER REOPEN")
                    return true;
            return false;
        }), "sending is wedged after close(): the in-flight packet was never cleared");
    }

    // the receive packet must not carry a truncated line across a reconnect
    void test_close_clears_partial_receive_packet()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        std::vector<std::string> received;
        auto recorder = [&received](const neolib::string_packet& aPacket)
        {
            received.push_back(packet_to_string(aPacket));
        };

        neolib::sink sink;
        sink += client.packet_arrived(recorder);
        connect(fix, client, sink, port);

        test_assert(fix.pump([&server]() { return server.last_stream() != nullptr; }), "server stream not available");
        server.last_stream()->send_packet(neolib::string_packet{ "TRUNCATED" });
        fix.settle();
        test_assert(received.empty(), "an unterminated line must not be delivered");

        client.close();
        fix.settle();

        neolib::sink sink2;
        sink2 += client.packet_arrived(recorder);
        connect(fix, client, sink2, port);

        test_assert(fix.pump([&server]() { return server.last_stream() != nullptr; }), "server stream not available");
        server.last_stream()->send_packet(neolib::string_packet{ "CLEAN\r\n" });

        test_assert(fix.pump([&received]() { return !received.empty(); }), "nothing arrived after reopening");
        test_assert(received[0] == "CLEAN", "close() left a truncated line in the receive packet");
    }

    // close() must send FIN, not RST. Closing a socket with a pending
    // overlapped read is an abortive close on Windows, which leaves the peer
    // reading connection_reset instead of eof; connection_reset is not
    // filtered by handle_read, so the peer raises a spurious TransferFailure
    // and records an error on an ordinary hangup.
    void test_close_delivers_eof_to_peer()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        connect(fix, client, sink, port);

        // leave a read outstanding on both ends, then hang up
        client.send_packet(neolib::string_packet{ "QUIT :bye\r\n" });
        test_assert(fix.pump([&server]() { return !server.received().empty(); }), "server never received the line");

        client.close();
        test_assert(fix.pump([&server]() { return server.connections_closed() >= 1u; }), "server never saw the close");
        fix.settle();

        test_assert(server.transfer_failures() == 0u,
            "peer raised a transfer failure on a clean hangup: " + server.last_transfer_failure());
        test_assert(!server.last_close_had_error(),
            "peer recorded an error on a clean hangup (expected eof, got " + server.last_close_error() + ")");
    }

    // and the same in the other direction: when the far end hangs up, the
    // local stream must end clean rather than with an error recorded against it
    void test_peer_close_delivers_eof_locally()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        std::uint32_t clientTransferFailures = 0u;
        std::string clientLastFailure;
        bool clientClosed = false;
        sink += client.transfer_failure([&](const boost::system::error_code& aError)
        {
            ++clientTransferFailures;
            clientLastFailure = describe(aError);
        });
        sink += client.connection_closed([&clientClosed]() { clientClosed = true; });

        {
            test_server<neolib::string_packet> server{ fix.task(), port };
            connect(fix, client, sink, port);

            client.send_packet(neolib::string_packet{ "PING :x\r\n" });
            test_assert(fix.pump([&server]() { return !server.received().empty(); }), "server never received the line");
        }
        // the server and its accepted stream are gone, closing the far end

        test_assert(fix.pump([&clientClosed]() { return clientClosed; }), "client never saw the peer hang up");
        test_assert(clientTransferFailures == 0u,
            "peer hangup raised a transfer failure: " + clientLastFailure);
        test_assert(!client.has_error(),
            "peer hangup recorded an error (expected eof, got " +
                std::to_string(client.error_code()) + " (" + client.error() + "))");
    }


    ///////////////////////////////////////////////////////////////////////////
    // TLS
    ///////////////////////////////////////////////////////////////////////////

    // self-signed, CN=localhost, SAN DNS:localhost + IP:127.0.0.1, valid to 2051
    char const sTestCertificate[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDJzCCAg+gAwIBAgIUBxaJnDq5cp6PcdPO5e+4IBJ5MQ4wDQYJKoZIhvcNAQEL\n"
        "BQAwFDESMBAGA1UEAwwJbG9jYWxob3N0MCAXDTI2MDkxOTEwNTg1MVoYDzIwNTEw\n"
        "NTExMTA1ODUxWjAUMRIwEAYDVQQDDAlsb2NhbGhvc3QwggEiMA0GCSqGSIb3DQEB\n"
        "AQUAA4IBDwAwggEKAoIBAQCYewrEztMPvbW6GtCvu+KOjmRpZik4Jh4YLf46JqSD\n"
        "N+mq6TqiDYt0KfeNgktTqJzbnmCIK9CLAinH9jUCCYHgRx6VY9X2EMPo9SnHVhEL\n"
        "FALU12dEmsWFdyw4QUWHRT9/KegwMW6BJVP2KOFtzV0f5E1Xbfe6KknC5oeSf5Ek\n"
        "vF1521Q7RpJkUBZHQqsD5eMMYfrh4B392lAabw5BjUDMEY9QWwj6vCtXsGUNNg/1\n"
        "wGa8xE7vD44tQIMcPHaoy6LkEwdfB4pSyZCShOV93Tap/DCRl5ExnJKSWREIvEJ/\n"
        "JBaxIirVlDBJCTCzb/Tfm/FcgJCY1Bmyp8Ou0OkOnLZ7AgMBAAGjbzBtMB0GA1Ud\n"
        "DgQWBBSY5WUUpbilSl5D/Qz/cP2jeV22HzAfBgNVHSMEGDAWgBSY5WUUpbilSl5D\n"
        "/Qz/cP2jeV22HzAPBgNVHRMBAf8EBTADAQH/MBoGA1UdEQQTMBGCCWxvY2FsaG9z\n"
        "dIcEfwAAATANBgkqhkiG9w0BAQsFAAOCAQEAK9kxoC1ZPW8xvKi7PaieGJvQdzHr\n"
        "n5JrV3ktrqiLVyK3NfIEswgqccv2knkbmK+fIpwmh2W+tQfwKb5yM2OekGdwCXdi\n"
        "tCqbh1zZpqw2XudfMMAwWMjRZJZu6h2qjIBuqtl2VINkqajETTYjhi1c6OSqIG0k\n"
        "0JQ7NxtgHzzDIcpa5sj5uMYZQel95s+nwSHOdKxRQbrWytQBUsFbA4hE/v896wW9\n"
        "kGblGQqEEcezr9SMVE7/nceoa4LAHLXQMsPYUS5XDnCXaAqDVhitaKRYieQww6SV\n"
        "jxErVZzXifSIobX+OMCKYKY+jouwM/IaabnImltm1Fm0wrbexaBM5oiqog==\n"
        "-----END CERTIFICATE-----\n";

    char const sTestPrivateKey[] =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCYewrEztMPvbW6\n"
        "GtCvu+KOjmRpZik4Jh4YLf46JqSDN+mq6TqiDYt0KfeNgktTqJzbnmCIK9CLAinH\n"
        "9jUCCYHgRx6VY9X2EMPo9SnHVhELFALU12dEmsWFdyw4QUWHRT9/KegwMW6BJVP2\n"
        "KOFtzV0f5E1Xbfe6KknC5oeSf5EkvF1521Q7RpJkUBZHQqsD5eMMYfrh4B392lAa\n"
        "bw5BjUDMEY9QWwj6vCtXsGUNNg/1wGa8xE7vD44tQIMcPHaoy6LkEwdfB4pSyZCS\n"
        "hOV93Tap/DCRl5ExnJKSWREIvEJ/JBaxIirVlDBJCTCzb/Tfm/FcgJCY1Bmyp8Ou\n"
        "0OkOnLZ7AgMBAAECggEAA0PUzrMF1opZR+nHRypupviW+NTJrcMCYo7CXB9nGjIw\n"
        "mP35r4ehUU4mviTbyEGBHlWNVQBPLL+enAJotY/B9EbBkzEgkm5e3rzR9X7UlzHV\n"
        "/69SR3Ti8g326FUP3Pe9uaAKZNT7vA//DlnNireng1I02VobwFfW3X5oyVldU8KD\n"
        "H3eSyNwWSXOU1Q5bNVdWMd3dGa9MokPImVBAdhJY2HnN/Y6GK7wCS0kJ7dnRyPds\n"
        "JxN46E46IsPSSAxDqCmq707V7VtdlulbrbQEt3elJajGPETYGsupNyuVaDylNH4c\n"
        "Sgj9Dv7HJzbW4ZgCSFmgS3PhJf/80vmPXW8Vrfk/JQKBgQDNQPAYYTql/AvvRcaT\n"
        "TRKAyxUv+1O076XiN7eCCRsZxdXFVEPcoWo6LdK9mpnSaBqFN+vhdtwtwwnIfUjW\n"
        "SGaJ8/MXE5Alus8IL72KBwAlwo724bw/CV/uW/CG7uHNpbz4ykymgH400IVTL5UV\n"
        "l81dbu4fs7a5V418lImo3xCpHQKBgQC+LfSn5bV2dguVrXFRUzsDnHwQwub7Z7NL\n"
        "swVy4IhjHJcG53k9tI46oMQOk12In+hV7QsSiJziiqg6CJrpSaj18/FgeHj0DvxE\n"
        "gwOgqVkdJOHoHanQpRilhTLkSgqABmxIZh4WbEiwLzxIHpK+q4Uf3yo9COHT6X1r\n"
        "bMye6NVidwKBgAd6mkUJJe3uZTDuxfGQGWCABeGdssshAFZh2VnvowEpaESscVyU\n"
        "tR4xlUA7Zed4y56XDw1EG3m/ZZfcmM7WDIZyUSVCHTqCuBCATNO+tY6qrDLqvsU9\n"
        "PjWdPAJuqeOoal5WYLygafjasED7tt72jREefabCEaJnQkyLzQhOOHkxAoGAf35R\n"
        "eMk/GJZUd9hZklqIwogCiD4RGVNQ2JvAOF4cMM121fMRzXMgsl8acGcMmk4RNKF/\n"
        "/cHF4v1vf9BLAcAW3CPYmoLJG1x8c/Wc1fURv779D13rfOthFX2xO2gDmAY2S2bi\n"
        "HsgjBrHz6KlOWTKlQVObfmVY93adVQoQNywB+UsCgYEAlJJ/RsEdw8vueIAon/Ti\n"
        "dRcPcFFo0GfZ/ik3uLH4ABA9Vdu3YeSR3BNMAOT4/7YOrFwdU+RI5A33gnNDR7bB\n"
        "sE2gPOlMZR0fQpmmwlOjnBv0Zbv6Ds3x6ZWH5S9NCl6bzgoKEUFHfPECAKNcqnsN\n"
        "0X/Fwl5IAUxiOv6idCfSsgo=\n"
        "-----END PRIVATE KEY-----\n";

    // a second self-signed certificate, valid only for wrong.example, used to
    // check that a trusted chain is not enough on its own
    char const sWrongHostCertificate[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDLTCCAhWgAwIBAgIUAfodW4m41FRO49UDheHmb/zKfbYwDQYJKoZIhvcNAQEL\n"
        "BQAwGDEWMBQGA1UEAwwNd3JvbmcuZXhhbXBsZTAgFw0yNjA5MTkxMTAwMzFaGA8y\n"
        "MDUxMDUxMTExMDAzMVowGDEWMBQGA1UEAwwNd3JvbmcuZXhhbXBsZTCCASIwDQYJ\n"
        "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAOhU6zElKz+2xNdscAJC+rqfd96cBgNl\n"
        "13k1QOZpbREUtLQ9cZj6wiKRHuSrEbylfJTnI8gXH5Q4KowQol6pwGRKEmXrtu7w\n"
        "9rcW69WTuFpodzXR+1eEaAPAq7lLMV2t67i6Y6HGUPZTgRsGmQgbhQSblR8XlT+2\n"
        "eJoijWRxNVEsNLKcgIUUOw5MkRVshSP38kJabID4LBzcxJaKqRMyofeHvAsGugid\n"
        "yfVcI7SjXtetnVYrTbyYeQt+JZRjIZYAsva/LHK9i+itqghL9+HoKWidkIoAcaQ8\n"
        "yspXC/2ztaodHlNTHuTRrShjEK3+gpS2kRuConEoyccoDSsItgY/0VMCAwEAAaNt\n"
        "MGswHQYDVR0OBBYEFMlg+bCRQvo30Ey86ZXt9yK/QZK3MB8GA1UdIwQYMBaAFMlg\n"
        "+bCRQvo30Ey86ZXt9yK/QZK3MA8GA1UdEwEB/wQFMAMBAf8wGAYDVR0RBBEwD4IN\n"
        "d3JvbmcuZXhhbXBsZTANBgkqhkiG9w0BAQsFAAOCAQEA5rAoZE2mlz92xZ1KeHZa\n"
        "kA5BiLxfG0Fr8b8t4bwnOD8FpKv76PCDVhPURgLccMQZtFLS2N4pGkMpaz9iCt2i\n"
        "VDOZ18nbVmGPdDWfT1g5XaCkTB+kTpD+Axc1VvVUWlHP15ECb7NzurxCWArvJqpr\n"
        "e4HGIPiKD6cp0GqEQtJ7SK18fQ/Gl2J2stH8KsUnfn8iejuCVkcWkNMz2kxVZyQH\n"
        "0IdQ9kTmgvlx6BL/+vVGbCc/iFztaA7cEXtDRv8cUpgOMKuZGNSu2C1Bvl4NIRnb\n"
        "6zfm8/WvhqHOdqrUMBqRLaLubB3CKu+KLWsz/pXr/ggf1GcqRNw9FNe5eXpB2/Ki\n"
        "ZA==\n"
        "-----END CERTIFICATE-----\n";

    char const sWrongHostPrivateKey[] =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDoVOsxJSs/tsTX\n"
        "bHACQvq6n3fenAYDZdd5NUDmaW0RFLS0PXGY+sIikR7kqxG8pXyU5yPIFx+UOCqM\n"
        "EKJeqcBkShJl67bu8Pa3FuvVk7haaHc10ftXhGgDwKu5SzFdreu4umOhxlD2U4Eb\n"
        "BpkIG4UEm5UfF5U/tniaIo1kcTVRLDSynICFFDsOTJEVbIUj9/JCWmyA+Cwc3MSW\n"
        "iqkTMqH3h7wLBroIncn1XCO0o17XrZ1WK028mHkLfiWUYyGWALL2vyxyvYvoraoI\n"
        "S/fh6ClonZCKAHGkPMrKVwv9s7WqHR5TUx7k0a0oYxCt/oKUtpEbgqJxKMnHKA0r\n"
        "CLYGP9FTAgMBAAECggEADZaR3uD2o5ZT8umDr44tTebvwtRLQb2eGZe0wQUffOVC\n"
        "IFyLnU/lNNJaBTPAKNFg9PCD1jsL/MZALsr2RCUXfIYch1t+6oCrXU/44Rfvq1H6\n"
        "zuoGEjtLPhTxtjegnoMCi15TX23S5GVD4snC/4dkgz16PkRJ1V0dtp4YuOwqGyIn\n"
        "MyBCa2ExrlUojx+k77kIaxh2T6j/pCTKkMuzJ1rCWhDCeXQov+cA3+qKEZ4siuw3\n"
        "4y7Nv3a92Y1fD+/mTeAlqHKDvCXDVM+CBOZ1jmZgs23qNSiB06IvOznSDUrcOV8P\n"
        "xqOTtgX4cOc+226Ht7o8kxiijlS6DzIlyXx6agJZSQKBgQD8zUBgRkcUVm+fAZoB\n"
        "yDHWyRM7yQLTp2YjxsAwtA6eC3FdymbG17z1lugkLTCGKbilRCXEIuZ4nt+0jrPL\n"
        "E/MkzahrDJcLKCOpeQDvLMn6ZpW1q0jO5RPSlrPGEXkuK5EAqgp1LuMiVUUC/fkN\n"
        "uyk6em8h36bqn3wxs70+8btCbwKBgQDrRV72IDzzdNF3GeLwZkL+Au/e5QUWZUWO\n"
        "34kFW72DxFdAHdo+H5i2hRiQ2aYhElsZ58PjnQCGHXW57z8eBSDjaNw8lFWwvBMy\n"
        "bGXJImNepBXFbQ//cIygeyoqzJOoRZ/gGjMRIDaki/Ee8Me75T49oMuHDJRYlNtH\n"
        "OFzFH43BXQKBgCKEE6oRblsEgjD/kvtzTfq5cXrUMyKa1INF+6+qpeRGQ7A+llHH\n"
        "2vDdLczirqFiyOcnqtBgw37skjag1UOA8c2wlrInAehwDo5xCwOc5ebeYspvHH3D\n"
        "ITW7hcghUp5PLHkevlbJlBF3+vgxnPOW/kYRuWKymqOmKtrOY6RYdOzZAoGBAJGw\n"
        "UICrL/M2iKtvHUfum1d+bBmjykW3Cp7Rr6Dg0XdyMvFiSw4jNMb5nl+8V5KtMjrV\n"
        "eUlOpM9oGMm97GDjnh0UhyUAWhvqKx0TYOhvYgduJokt4zUz9fE+s7rzGhCepMT6\n"
        "lFrrjsrQWczmH1ksOSGim8YSNR6xdyQgM9phkr+FAoGBAM6yi5lZlu/86L7l169V\n"
        "RLGmTinKQwPoQyw751/Uqv+k9d+kb5FPD3ezSxpYXXAd9/SrVlYKSAlQH3tD8yng\n"
        "S1of4VyRF/q0o6p6aY/YELD0DL7c47UW4yq5UgQTMkqz8ZilnTyluN1upE5F3Htw\n"
        "fkm/OfPEJ4Juo+xMpe0WX5sl\n"
        "-----END PRIVATE KEY-----\n";

    void configure_test_server_certificate(boost::asio::ssl::context& aContext)
    {
        aContext.use_certificate_chain(boost::asio::buffer(sTestCertificate, sizeof(sTestCertificate) - 1));
        aContext.use_private_key(
            boost::asio::buffer(sTestPrivateKey, sizeof(sTestPrivateKey) - 1),
            boost::asio::ssl::context::pem);
    }

    // the certificate has to be installed before the first accept, so the
    // context is built up front and handed to the server
    template <typename PacketType>
    std::unique_ptr<test_server<PacketType>> make_secure_test_server(neolib::async_task& aTask, unsigned short aPort, bool aEcho = false)
    {
        auto context = test_server<PacketType>::stream_type::connection_type::create_secure_context(true);
        configure_test_server_certificate(*context);
        return std::make_unique<test_server<PacketType>>(aTask, aPort, aEcho, context);
    }

    // the client trusts our test CA and nothing else, so a successful handshake
    // means the certificate really was verified
    void trust_test_certificate(neolib::tcp_string_packet_stream& aClient)
    {
        aClient.connection().secure_context().add_certificate_authority(
            boost::asio::buffer(sTestCertificate, sizeof(sTestCertificate) - 1));
    }

    void test_tls_round_trip()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        auto server = make_secure_test_server<neolib::string_packet>(fix.task(), port, true);

        neolib::tcp_string_packet_stream client{ fix.task(), true };
        trust_test_certificate(client);

        neolib::sink sink;
        std::vector<std::string> echoed;
        std::string clientFailure;
        sink += client.packet_arrived([&echoed](const neolib::string_packet& aPacket)
        {
            echoed.push_back(packet_to_string(aPacket));
        });
        sink += client.connection_failure([&clientFailure](const boost::system::error_code& aError)
        {
            clientFailure = describe(aError);
        });

        bool connected = false;
        sink += client.connection_established([&connected]() { connected = true; });
        client.open("localhost", port, true);
        test_assert(fix.pump([&connected]() { return connected; }), "TLS handshake failed: " + clientFailure);

        client.send_packet(neolib::string_packet{ "NICK tester\r\n" });
        test_assert(fix.pump([&server]() { return !server->received().empty(); }), "server received nothing over TLS");
        test_assert(server->received()[0] == "NICK tester", "TLS packet contents");
        test_assert(fix.pump([&echoed]() { return !echoed.empty(); }), "client received no echo over TLS");
        test_assert(echoed[0] == "NICK tester", "TLS echo contents");
        test_assert(server->transfer_failures() == 0u, "unexpected transfer failure: " + server->last_transfer_failure());
    }

    // an untrusted certificate must fail the handshake rather than be accepted
    // silently, which is what verify_none did
    void test_tls_rejects_untrusted_certificate()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        auto server = make_secure_test_server<neolib::string_packet>(fix.task(), port);

        // no trust_test_certificate() here: the default context verifies
        // against the system CA store, which has never heard of our test cert
        neolib::tcp_string_packet_stream client{ fix.task(), true };

        neolib::sink sink;
        bool connected = false;
        bool failed = false;
        sink += client.connection_established([&connected]() { connected = true; });
        sink += client.connection_failure([&failed](const boost::system::error_code&) { failed = true; });

        client.open("localhost", port, true);

        test_assert(fix.pump([&]() { return connected || failed; }), "handshake neither succeeded nor failed");
        test_assert(!connected, "an untrusted certificate was accepted");
        test_assert(failed, "an untrusted certificate did not raise connection_failure");
    }

    // a trusted chain is not enough on its own: the server presents a
    // certificate the client trusts as a CA but which is issued for
    // wrong.example, so connecting to localhost must fail host name verification
    void test_tls_rejects_wrong_host_name()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());

        auto context = neolib::tcp_string_packet_stream::connection_type::create_secure_context(true);
        context->use_certificate_chain(boost::asio::buffer(sWrongHostCertificate, sizeof(sWrongHostCertificate) - 1));
        context->use_private_key(
            boost::asio::buffer(sWrongHostPrivateKey, sizeof(sWrongHostPrivateKey) - 1),
            boost::asio::ssl::context::pem);
        test_server<neolib::string_packet> server{ fix.task(), port, false, context };

        neolib::tcp_string_packet_stream client{ fix.task(), true };
        client.connection().secure_context().add_certificate_authority(
            boost::asio::buffer(sWrongHostCertificate, sizeof(sWrongHostCertificate) - 1));

        neolib::sink sink;
        bool connected = false;
        bool failed = false;
        sink += client.connection_established([&connected]() { connected = true; });
        sink += client.connection_failure([&failed](const boost::system::error_code&) { failed = true; });

        client.open("localhost", port, true);

        test_assert(fix.pump([&]() { return connected || failed; }), "handshake neither succeeded nor failed");
        test_assert(!connected, "a certificate issued for another host name was accepted");
        test_assert(failed, "host name mismatch did not raise connection_failure");
    }

    // a TLS peer that drops the transport without close_notify yields
    // ssl::error::stream_truncated, which must be treated as end of stream
    void test_tls_close_is_not_a_transfer_failure()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        auto server = make_secure_test_server<neolib::string_packet>(fix.task(), port);

        neolib::tcp_string_packet_stream client{ fix.task(), true };
        trust_test_certificate(client);

        neolib::sink sink;
        std::uint32_t clientTransferFailures = 0u;
        std::string clientLastFailure;
        bool clientClosed = false;
        sink += client.transfer_failure([&](const boost::system::error_code& aError)
        {
            ++clientTransferFailures;
            clientLastFailure = describe(aError);
        });
        sink += client.connection_closed([&clientClosed]() { clientClosed = true; });

        bool connected = false;
        sink += client.connection_established([&connected]() { connected = true; });
        client.open("localhost", port, true);
        test_assert(fix.pump([&connected]() { return connected; }), "TLS handshake failed");

        client.send_packet(neolib::string_packet{ "PING :x\r\n" });
        test_assert(fix.pump([&server]() { return !server->received().empty(); }), "server received nothing over TLS");

        server.reset(); // far end hangs up

        test_assert(fix.pump([&clientClosed]() { return clientClosed; }), "client never saw the TLS peer hang up");
        test_assert(clientTransferFailures == 0u,
            "TLS hangup raised a transfer failure: " + clientLastFailure);
        test_assert(!client.has_error(),
            "TLS hangup recorded an error: " + std::to_string(client.error_code()) + " (" + client.error() + ")");
    }

    // the server's own connection_closed slot is registered before the stream is
    // handed out, and it is what removes the stream. If it destroys the stream
    // inline, the event dies mid-iteration and every slot an application added
    // afterwards is silently skipped.
    void test_server_stream_connection_closed_is_reachable()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };
        neolib::sink sink;
        connect(fix, client, sink, port);
        test_assert(fix.pump([&server]() { return server.connections() >= 1u; }), "server did not accept the connection");

        client.close();

        test_assert(fix.pump([&server]() { return server.stream_connection_closed() >= 1u; }),
            "a slot on the server-side stream's connection_closed was never called");
        test_assert(server.connections_closed() >= 1u, "packet_stream_removed did not fire");
    }

    void test_repeated_reconnect_cycles()
    {
        io_fixture fix;
        auto const port = find_free_port(fix.task());
        test_server<neolib::string_packet> server{ fix.task(), port };

        neolib::tcp_string_packet_stream client{ fix.task() };

        for (int cycle = 0; cycle != 5; ++cycle)
        {
            neolib::sink sink;
            connect(fix, client, sink, port);

            std::string const line = "CYCLE " + std::to_string(cycle);
            client.send_packet(neolib::string_packet{ line + "\r\n" });

            test_assert(fix.pump([&server, cycle]()
            {
                return server.received().size() >= static_cast<std::size_t>(cycle) + 1u;
            }), "packet lost on reconnect cycle");
            test_assert(server.received().back() == line, "wrong packet after reconnect");

            client.close();
            test_assert(fix.pump([&server, cycle]()
            {
                return server.connections_closed() >= static_cast<std::uint32_t>(cycle) + 1u;
            }), "server never saw the close");
        }

        test_assert(server.received().size() == 5u, "stale or duplicated packets across reconnects");
        test_assert(server.transfer_failures() == 0u, "unexpected transfer failure across reconnects: " + server.last_transfer_failure());
    }
}

int main()
{
    try
    {
        neolib::allocate_service_provider();
    }
    catch (const std::exception& e)
    {
        std::cout << "failed to start services: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "neolib::io packet tests" << std::endl;
    run_test("string_packet: CRLF lines", test_string_packet_crlf_lines);
    run_test("string_packet: split across reads", test_string_packet_split_across_reads);
    run_test("string_packet: empty range", test_string_packet_empty_range);
    run_test("string_packet: leading delimiters", test_string_packet_leading_delimiters);
    run_test("string_packet: LF-only line endings", test_string_packet_lf_only_line_endings);
    run_test("string_packet: trailing CR", test_string_packet_trailing_cr);
    run_test("string_packet: long line accumulation", test_string_packet_accumulates_long_line);
    run_test("string_packet: data/clone/copy_from", test_string_packet_data_clone_copy);
    run_test("binary_packet: take_some", test_binary_packet_take_some);

    std::cout << "neolib::io loopback tests" << std::endl;
    run_test("loopback: round trip", test_loopback_round_trip);
    run_test("loopback: packets sharing one write", test_loopback_packets_share_one_write);
    run_test("loopback: line larger than receive buffer", test_loopback_line_larger_than_receive_buffer);
    run_test("loopback: unterminated line withheld", test_loopback_unterminated_line_is_withheld);
    run_test("loopback: clean close is not a transfer failure", test_loopback_clean_close_is_not_a_transfer_failure);
    run_test("loopback: connection failure to dead port", test_connection_failure_to_dead_port);
    run_test("loopback: binary stream round trip", test_binary_stream_round_trip);

    std::cout << "neolib::io close()/reconnect regression tests" << std::endl;
    run_test("close(): drops queued packets", test_close_drops_queued_packets);
    run_test("close(): clears in-flight packet", test_close_clears_packet_being_sent);
    run_test("close(): clears partial receive packet", test_close_clears_partial_receive_packet);
    run_test("close(): delivers eof to the peer", test_close_delivers_eof_to_peer);
    run_test("close(): peer hangup delivers eof locally", test_peer_close_delivers_eof_locally);
    run_test("close(): repeated reconnect cycles", test_repeated_reconnect_cycles);
    run_test("server: stream connection_closed is reachable", test_server_stream_connection_closed_is_reachable);

    std::cout << "neolib::io TLS tests" << std::endl;
    run_test("tls: round trip", test_tls_round_trip);
    run_test("tls: rejects untrusted certificate", test_tls_rejects_untrusted_certificate);
    run_test("tls: rejects wrong host name", test_tls_rejects_wrong_host_name);
    run_test("tls: peer hangup is not a transfer failure", test_tls_close_is_not_a_transfer_failure);

    if (sFailures != 0u)
    {
        std::cout << sFailures << " test(s) failed" << std::endl;
        // exiting by exception terminates the process, which CTest reports as
        // "Exception" and which hides everything printed above
        return EXIT_FAILURE;
    }

    std::cout << "all tests passed" << std::endl;
    return EXIT_SUCCESS;
}
