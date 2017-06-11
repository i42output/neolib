// tcp_packet_stream_server.hpp
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
#include <memory>
#include <boost/asio.hpp>
#include "io_task.hpp"
#include "observable.hpp"
#include "packet_stream.hpp"

namespace neolib
{
	template <typename PacketType>
	class tcp_packet_stream_server;

	template <typename PacketType>
	class i_tcp_packet_stream_server_observer
	{
		// types
	public:
		typedef PacketType packet_type;
		typedef tcp_protocol protocol_type;
		typedef tcp_packet_stream_server<packet_type> tcp_packet_stream_server_type;
		typedef packet_stream<packet_type, protocol_type> packet_stream_type;

		// interface
	public:
		virtual void packet_stream_added(tcp_packet_stream_server_type& aServer, packet_stream_type& aStream) = 0;
		virtual void packet_stream_removed(tcp_packet_stream_server_type& aServer, packet_stream_type& aStream) = 0;
		virtual void failed_to_accept_packet_stream(tcp_packet_stream_server_type& aServer, const boost::system::error_code& aError) = 0;

		// types
	public:
		enum notify_type 
		{ 
			NotifyPacketStreamAdded, 
			NotifyPacketStreamRemoved,
			NotifyFailedToAcceptPacketStream
		};
	};

	template <typename PacketType>
	class tcp_packet_stream_server : public observable<i_tcp_packet_stream_server_observer<PacketType> >, private i_packet_stream_observer<PacketType, tcp_protocol>
	{
		// types
	public:
		typedef tcp_packet_stream_server<PacketType> our_type;
		typedef PacketType packet_type;
		typedef tcp_protocol protocol_type;
		typedef i_tcp_packet_stream_server_observer<packet_type> observer_type;
		typedef packet_stream<packet_type, protocol_type> packet_stream_type;
		typedef typename packet_stream_type::pointer packet_stream_pointer;
		typedef std::vector<packet_stream_type*> stream_list;
		typedef protocol_type::endpoint endpoint_type;
		typedef protocol_type::resolver resolver_type;
		typedef protocol_type::acceptor acceptor_type;
	private:
		class handler_proxy
		{
		public:
			handler_proxy(our_type& aParent) : iParent(aParent), iOrphaned(false)
			{
			}
		public:
			void operator()(const boost::system::error_code& aError)
			{
				if (!iOrphaned)
					iParent.handle_accept(aError);
			}
			void orphan(bool aCreateNewHandlerProxy = true)
			{
				iOrphaned = true;
				if (aCreateNewHandlerProxy)
					iParent.iHandlerProxy = std::make_shared<handler_proxy>(iParent);
				else
					iParent.iHandlerProxy.reset();
			}
		private:
			our_type& iParent;
			bool iOrphaned;
		};

		// exceptions
	public:
		struct failed_to_resolve_local_host : std::runtime_error { failed_to_resolve_local_host() : std::runtime_error("neolib::tcp_packet_stream_server::failed_to_resolve_local_host") {} };
		struct stream_not_found : std::logic_error { stream_not_found() : std::logic_error("neolib::tcp_packet_stream_server::stream_not_found") {} };

		// construction
	public:
		tcp_packet_stream_server(io_task& aIoTask, unsigned short aLocalPort, bool aSecure = false, protocol_family aProtocolFamily = IPv4) :
			iIoTask(aIoTask),
			iHandlerProxy(new handler_proxy(*this)),
			iLocalPort(aLocalPort),
			iSecure(aSecure),
			iProtocolFamily(aProtocolFamily & IPv4 ? protocol_type::v4() : protocol_type::v6()),
			iLocalEndpoint(iProtocolFamily, iLocalPort),
			iAcceptor(aIoTask.networking_io_service().native_object(), iLocalEndpoint), 
			iClosing(false)
		{
			accept_connection();
		}
		tcp_packet_stream_server(io_task& aIoTask, const std::string& aLocalHostName, unsigned short aLocalPort, bool aSecure = false, protocol_family aProtocolFamily = IPv4) :
			iIoTask(aIoTask),
			iHandlerProxy(new handler_proxy(*this)),
			iLocalHostName(aLocalHostName),
			iLocalPort(aLocalPort),
			iSecure(aSecure),
			iProtocolFamily(aProtocolFamily & IPv4 ? protocol_type::v4() : protocol_type::v6()),
			iLocalEndpoint(resolve(aIoTask, iLocalHostName, iLocalPort, iProtocolFamily)),
			iAcceptor(aIoTask.networking_io_service().native_object(), iLocalEndpoint),
			iClosing(false)
		{
			accept_connection();
		}
		~tcp_packet_stream_server()
		{
			iClosing = true;
			for (stream_list::iterator i = iStreamList.begin(); i != iStreamList.end(); ++i)
				delete *i;
			iStreamList.clear();
			iHandlerProxy->orphan();
			iAcceptor.close();
		}
		
		// operations
	public:
		unsigned short local_port() const
		{
			return iLocalPort;
		}
		packet_stream_pointer take_ownership(packet_stream_type& aStream)
		{
			for (stream_list::iterator i = iStreamList.begin(); i != iStreamList.end(); ++i)
				if (*i == &aStream)
				{
					packet_stream_pointer found(*i);
					iStreamList.erase(i);
					aStream.remove_observer(*this);
					return found;
				}
			throw stream_not_found();
		}
		
		// implementation
	private:
		// from observable<i_tcp_packet_stream_server_observer<PacketType> >
		virtual void notify_observer(observer_type& aObserver, typename observer_type::notify_type aType, const void* aParameter, const void*)
		{
			switch(aType)
			{
			case observer_type::NotifyPacketStreamAdded:
				aObserver.packet_stream_added(*this, *static_cast<packet_stream_type*>(const_cast<void*>(aParameter)));
				break;
			case observer_type::NotifyPacketStreamRemoved:
				aObserver.packet_stream_removed(*this, *static_cast<packet_stream_type*>(const_cast<void*>(aParameter)));
				break;
			case observer_type::NotifyFailedToAcceptPacketStream:
				aObserver.failed_to_accept_packet_stream(*this, *static_cast<const boost::system::error_code*>(aParameter));
				break;
			}
		}
		// from i_packet_stream_observer<PacketType, tcp_protocol>
		virtual void connection_established(packet_stream_type&) {}
		virtual void connection_failure(packet_stream_type&, const boost::system::error_code&) {}
		virtual void packet_sent(packet_stream_type&, const packet_type&) {}
		virtual void packet_arrived(packet_stream_type&, const packet_type&) {}
		virtual void transfer_failure(packet_stream_type&, const boost::system::error_code&) {}
		virtual void connection_closed(packet_stream_type& aStream)
		{
			if (!iClosing)
			{
				for (typename stream_list::iterator i = iStreamList.begin(); i != iStreamList.end(); ++i)
					if (&**i == &aStream)
					{
						packet_stream_pointer closingStream(*i);
						iStreamList.erase(i);
						notify_observers(observer_type::NotifyPacketStreamRemoved, *closingStream);
						break;
					}
			}
			else
				notify_observers(observer_type::NotifyPacketStreamRemoved, aStream);
		}
		// own
	private:
		static endpoint_type resolve(io_task& aIoTask, const std::string& aHostname, unsigned short aPort, protocol_type aProtocolFamily)
		{
			resolver_type resolver(aIoTask.networking_io_service().native_object());
			boost::system::error_code ec;
			typename resolver_type::iterator result = resolver.resolve(resolver_type::query(aHostname, unsigned_integer_to_string<char>(aPort)), ec);
			if (!ec)
			{
				for (typename resolver_type::iterator i = result; i != resolver_type::iterator(); ++i)
				{
					endpoint_type endpoint = *i;
					if (endpoint.protocol() == aProtocolFamily)
						return endpoint;
				}
				return *result;
			}
			throw failed_to_resolve_local_host();
		}
		void accept_connection()
		{
			if (iAcceptingStream != nullptr)
				return;
			iAcceptingStream = packet_stream_pointer(new packet_stream_type(iIoTask, iSecure, iLocalEndpoint.protocol() == protocol_type::v4() ? IPv4 : IPv6));
			iAcceptingStream->add_observer(*this);
			iAcceptingStream->connection().open(true);
			iAcceptor.async_accept(iAcceptingStream->connection().socket(), boost::bind(&handler_proxy::operator(), iHandlerProxy, boost::asio::placeholders::error));
		}
		void handle_accept(const boost::system::error_code& aError)
		{
			if (!aError)
			{
				iAcceptingStream->connection().server_accept();
				iStreamList.push_back(iAcceptingStream.get());
				iAcceptingStream.release();
				notify_observers(observer_type::NotifyPacketStreamAdded, *iStreamList.back());
				accept_connection();
			}
			else
			{
				std::string m = aError.message();
				notify_observers(observer_type::NotifyFailedToAcceptPacketStream, aError);
			}
		}
		
		// attributes
	private:
		io_task& iIoTask;
		std::shared_ptr<handler_proxy> iHandlerProxy;
		std::string iLocalHostName;
		unsigned short iLocalPort;
		bool iSecure;
		protocol_type iProtocolFamily;
		endpoint_type iLocalEndpoint;
		acceptor_type iAcceptor;
		packet_stream_pointer iAcceptingStream;
		stream_list iStreamList;
		bool iClosing;
	};

	typedef tcp_packet_stream_server<string_packet> tcp_string_packet_stream_server;
	typedef i_tcp_packet_stream_server_observer<string_packet> tcp_string_packet_stream_server_observer;
}
