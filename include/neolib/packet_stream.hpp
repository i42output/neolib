// packet_stream.hpp
/*
 *  Copyright (c) 2012 Leigh Johnston.
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

#include "neolib.hpp"
#include <stdexcept>
#include <vector>
#include "io_thread.hpp"
#include "observable.hpp"
#include "i_packet.hpp"
#include "binary_packet.hpp"
#include "string_packet.hpp"
#include "packet_connection.hpp"

namespace neolib
{
	template <typename PacketType, typename Protocol>
	class packet_stream;

	template <typename PacketType, typename Protocol>
	class i_packet_stream_observer
	{
		// types
	public:
		typedef PacketType packet_type;
		typedef Protocol protocol_type;
		typedef packet_stream<packet_type, protocol_type> packet_stream_type;

		// interface
	public:
		virtual void connection_established(packet_stream_type& aStream) = 0;
		virtual void connection_failure(packet_stream_type& aStream, const boost::system::error_code& aError) = 0;
		virtual void packet_sent(packet_stream_type& aStream, const packet_type& aPacket) = 0;
		virtual void packet_arrived(packet_stream_type& aStream, const packet_type& aPacket) = 0;
		virtual void transfer_failure(packet_stream_type& aStream, const boost::system::error_code& aError) = 0;
		virtual void connection_closed(packet_stream_type& aStream) = 0;

		// types
	public:
		enum notify_type 
		{ 
			NotifyConnectionEstablished, 
			NotifyConnectionFailure, 
			NotifyPacketSent,
			NotifyPacketArrived,
			NotifyTransferFailure,
			NotifyConnectionClosed
		};
	};

	template <typename PacketType, typename Protocol>
	class packet_stream : 
		public observable<i_packet_stream_observer<PacketType, Protocol> >,
		private i_basic_packet_connection_owner<typename PacketType::character_type>
	{
		// types
	public:
		typedef PacketType packet_type;
		typedef Protocol protocol_type;
		typedef std::unique_ptr<packet_stream> pointer;
		typedef i_basic_packet<typename packet_type::character_type> generic_packet_type;
		typedef typename packet_type::clone_pointer packet_clone_pointer;
		typedef i_packet_stream_observer<packet_type, protocol_type> observer_type;
		typedef basic_packet_connection<typename packet_type::character_type, Protocol> connection_type;
		typedef packet_type* queue_item;
		typedef std::unique_ptr<packet_type> orphaned_queue_item;
		typedef std::vector<queue_item> send_queue;
		
		// construction
	public:
		packet_stream(io_thread& aOwnerThread, bool aSecure = false, protocol_family aProtocolFamily = IPv4) : 
			iConnection(aOwnerThread, *this, aSecure, aProtocolFamily)
		{
		}
		packet_stream(io_thread& aOwnerThread, const std::string& aHostName, unsigned short aPort, bool aSecure = false, protocol_family aProtocolFamily = IPv4) : 
			iConnection(aOwnerThread, *this, aHostName, aPort, aSecure, aProtocolFamily)
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
			orphaned_queue_item newPacket(new packet_type(aPacket));
			iSendQueue.push_back(newPacket.get());
			iConnection.send_packet(*newPacket.release(), aHighPriority);
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
		// from observable<i_packet_stream_observer<PacketType, Protocol> >
		virtual void notify_observer(observer_type& aObserver, typename observer_type::notify_type aType, const void* aParameter, const void* aParameter2)
		{
			switch(aType)
			{
			case observer_type::NotifyConnectionEstablished:
				aObserver.connection_established(*this);
				break;
			case observer_type::NotifyConnectionFailure:
				aObserver.connection_failure(*this, *static_cast<const boost::system::error_code*>(aParameter));
				break;
			case observer_type::NotifyPacketSent:
				aObserver.packet_sent(*this, *static_cast<const packet_type*>(aParameter));
				break;
			case observer_type::NotifyPacketArrived:
				aObserver.packet_arrived(*this, *static_cast<const packet_type*>(aParameter));
				break;
			case observer_type::NotifyTransferFailure:
				aObserver.transfer_failure(*this, *static_cast<const boost::system::error_code*>(aParameter));
				break;
			case observer_type::NotifyConnectionClosed:
				aObserver.connection_closed(*this);
				break;
			}
		}
		// from i_basic_packet_connection_owner<typename PacketType::character_type>
		virtual packet_clone_pointer create_empty_packet() const
		{
			return packet_clone_pointer(new packet_type());
		}
		virtual void connection_established()
		{
			notify_observers(observer_type::NotifyConnectionEstablished);
		}
		virtual void connection_failure(const boost::system::error_code& aError)
		{
			notify_observers(observer_type::NotifyConnectionFailure, aError);
		}
		virtual void packet_sent(const generic_packet_type& aPacket)
		{
			orphaned_queue_item sentPacket = remove_packet(static_cast<const packet_type&>(aPacket));
			notify_observers(observer_type::NotifyPacketSent, *sentPacket);
		}
		virtual void packet_arrived(const generic_packet_type& aPacket)
		{
			notify_observers(observer_type::NotifyPacketArrived, static_cast<const packet_type&>(aPacket));
		}
		virtual void transfer_failure(const generic_packet_type& aPacket, const boost::system::error_code& aError)
		{
			orphaned_queue_item failedPacket = remove_packet(static_cast<const packet_type&>(aPacket));
			notify_observers(observer_type::NotifyTransferFailure, aError);
		}
		virtual void connection_closed()
		{
			notify_observers(observer_type::NotifyConnectionClosed);
		}
		orphaned_queue_item remove_packet(const packet_type& aPacket)
		{
			orphaned_queue_item removedPacket;
			for (send_queue::iterator i = iSendQueue.begin(); i != iSendQueue.end(); ++i)
				if (*i == &aPacket)
				{
					removedPacket.reset(*i);
					iSendQueue.erase(i);
					return removedPacket;
				}
			return removedPacket;
		}
		void remove_all_packets()
		{
			for (send_queue::iterator i = iSendQueue.begin(); i != iSendQueue.end(); ++i)
			{
				delete *i;
				*i = 0;
			}
			iSendQueue.clear();
		}
		// attributes
	private:
		send_queue iSendQueue;
		connection_type iConnection;
	};

	typedef i_packet_stream_observer<binary_packet, tcp_protocol> i_tcp_binary_packet_stream_observer;
	typedef packet_stream<binary_packet, tcp_protocol> tcp_binary_packet_stream;
	typedef i_packet_stream_observer<string_packet, tcp_protocol> i_tcp_string_packet_stream_observer;
	typedef packet_stream<string_packet, tcp_protocol> tcp_string_packet_stream;
}
