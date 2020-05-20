// packet_stream.hpp
/*
 *  Copyright (c) 2007 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <stdexcept>
#include <vector>
#include <neolib/task/async_task.hpp>
#include <neolib/plugin/plugin_event.hpp>
#include <neolib/io/i_packet.hpp>
#include <neolib/io/binary_packet.hpp>
#include <neolib/io/string_packet.hpp>
#include <neolib/io/packet_connection.hpp>

namespace neolib
{
    template <typename PacketType, typename Protocol>
    class packet_stream;

    template <typename PacketType, typename Protocol>
    class packet_stream : private i_basic_packet_connection_owner<typename PacketType::character_type>
    {
        // types
    public:
        typedef PacketType packet_type;
        typedef Protocol protocol_type;
        typedef std::unique_ptr<packet_stream> pointer;
        typedef i_basic_packet<typename packet_type::character_type> generic_packet_type;
        typedef typename packet_type::clone_pointer packet_clone_pointer;
        typedef basic_packet_connection<typename packet_type::character_type, Protocol> connection_type;
        typedef std::unique_ptr<packet_type> queue_item;
        typedef std::unique_ptr<packet_type> orphaned_queue_item;
        typedef std::vector<queue_item> send_queue;
        
        // events
    public:
        define_event(ConnectionEstablished, connection_established)
        define_event(ConnectionFailure, connection_failure, const boost::system::error_code&)
        define_event(PacketSent, packet_sent, const packet_type&)
        define_event(PacketArrived, packet_arrived, const packet_type&)
        define_event(TransferFailure, transfer_failure, const boost::system::error_code&)
        define_event(ConnectionClosed, connection_closed)

        // construction
    public:
        packet_stream(async_task& aIoTask, bool aSecure = false, protocol_family aProtocolFamily = IPv4) : 
            iConnection(aIoTask, *this, aSecure, aProtocolFamily)
        {
        }
        packet_stream(async_task& aIoTask, const std::string& aHostName, unsigned short aPort, bool aSecure = false, protocol_family aProtocolFamily = IPv4) :
            iConnection(aIoTask, *this, aHostName, aPort, aSecure, aProtocolFamily)
        {
        }
        ~packet_stream()
        {
            remove_all_packets();
        }
        packet_stream(const packet_stream& aRhs) = delete;
        packet_stream& operator=(const packet_stream& aRhs) = delete;
        
        // operations
    public:
        bool open(const std::string& aRemoteHostName, unsigned short aRemotePort, bool aSecure = false, protocol_family aProtocolFamily = IPv4)
        {
            return iConnection.open(aRemoteHostName, aRemotePort, aSecure, aProtocolFamily);
        }
        bool opened() const
        {
            return iConnection.opened();
        }
        void close()
        {
            remove_all_packets();
            iConnection.close();
        }
        void send_packet(const packet_type& aPacket, bool aHighPriority = false)
        {
            iSendQueue.push_back(std::make_unique<packet_type>(aPacket));
            iConnection.send_packet(*iSendQueue.back(), aHighPriority);
        }
        bool connected() const
        {
            return iConnection.connected();
        }
        bool closed() const
        {
            return iConnection.closed();
        }
        bool has_error() const
        {
            return iConnection.has_error();
        }
        std::string error() const
        {
            return iConnection.error().message();
        }
        int error_code() const
        {
            return iConnection.error().value();
        }
        const connection_type& connection() const
        {
            return iConnection;
        }
        connection_type& connection()
        {
            return iConnection;
        }
        bool underflow() const
        {
            return iSendQueue.empty();
        }
        
        // implementation
    private:
        // from i_basic_packet_connection_owner<typename PacketType::character_type>
        packet_clone_pointer handle_create_empty_packet() const override
        {
            return packet_clone_pointer(new packet_type());
        }
        void handle_connection_established() override
        {
            ConnectionEstablished.trigger();
        }
        void handle_connection_failure(const boost::system::error_code& aError) override
        {
            ConnectionFailure.trigger(aError);
        }
        void handle_packet_sent(const generic_packet_type& aPacket) override
        {
            orphaned_queue_item sentPacket = remove_packet(static_cast<const packet_type&>(aPacket));
            PacketSent.trigger(*sentPacket);
        }
        void handle_packet_arrived(const generic_packet_type& aPacket) override
        {
            PacketArrived.trigger(static_cast<const packet_type&>(aPacket));
        }
        void handle_transfer_failure(const generic_packet_type& aPacket, const boost::system::error_code& aError) override
        {
            orphaned_queue_item failedPacket = remove_packet(static_cast<const packet_type&>(aPacket));
            TransferFailure.trigger(aError);
        }
        void handle_connection_closed() override
        {
            ConnectionClosed.trigger();
        }
        orphaned_queue_item remove_packet(const packet_type& aPacket)
        {
            orphaned_queue_item removedPacket;
            for (auto i = iSendQueue.begin(); i != iSendQueue.end(); ++i)
                if (&**i == &aPacket)
                {
                    removedPacket = std::move(*i);
                    iSendQueue.erase(i);
                    return removedPacket;
                }
            return removedPacket;
        }
        void remove_all_packets()
        {
            iSendQueue.clear();
        }
        // attributes
    private:
        send_queue iSendQueue;
        connection_type iConnection;
    };

    typedef packet_stream<binary_packet, tcp_protocol> tcp_binary_packet_stream;
    typedef packet_stream<string_packet, tcp_protocol> tcp_string_packet_stream;
}
