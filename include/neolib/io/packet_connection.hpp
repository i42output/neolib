// packet_connection.hpp
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
#include <memory>
#include <deque>
#include <array>
#include <algorithm>
#include <variant>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/core/lifetime.hpp>
#include <neolib/task/async_task.hpp>
#include <neolib/io/resolver.hpp> // protocol_family
#include <neolib/io/i_packet.hpp>

namespace neolib
{
    typedef boost::asio::ip::tcp tcp_protocol;
    typedef boost::asio::ip::udp udp_protocol;

    template <typename CharType>
    class i_basic_packet_connection_owner
    {
        // types
    public:
        typedef i_basic_packet<CharType> packet_type;
        typedef typename packet_type::clone_pointer packet_clone_pointer;
        // interface
    public:
        virtual packet_clone_pointer handle_create_empty_packet() const = 0;
        virtual void handle_connection_established() = 0;
        virtual void handle_connection_failure(const boost::system::error_code& aError) = 0;
        virtual void handle_packet_sent(const packet_type& aPacket) = 0;
        virtual void handle_packet_arrived(const packet_type& aPacket) = 0;
        virtual void handle_transfer_failure(const packet_type& aPacket, const boost::system::error_code& aError) = 0;
        virtual void handle_connection_closed() = 0;
    };

    typedef i_basic_packet_connection_owner<char> packet_connection_owner;

    template <typename CharType, typename Protocol, size_t ReceiveBufferSize = 1024>
    class basic_packet_connection : public lifetime<>
    {
        // types
    public:
        typedef Protocol protocol_type;
    private:
        typedef basic_packet_connection<CharType, Protocol, ReceiveBufferSize> our_type;
        typedef i_basic_packet_connection_owner<CharType> owner_type;
        typedef i_basic_packet<CharType> packet_type;
        typedef const packet_type* const_packet_pointer;
        typedef std::deque<const_packet_pointer> send_queue;
        typedef typename protocol_type::socket socket_type;
        typedef std::shared_ptr<socket_type> socket_pointer;
        typedef boost::asio::ssl::stream<tcp_protocol::socket> secure_stream_type;
        typedef std::shared_ptr<secure_stream_type> secure_stream_pointer;
        typedef std::variant<std::monostate, socket_pointer, secure_stream_pointer> socket_holder_type;
        typedef boost::asio::ssl::context secure_stream_context;
        typedef std::unique_ptr<secure_stream_context> secure_stream_context_pointer;
        typedef typename protocol_type::endpoint endpoint_type;
        typedef typename protocol_type::resolver resolver_type;
        typedef std::array<char, ReceiveBufferSize * sizeof(CharType)> receive_buffer;
        typedef boost::asio::ssl::context secure_context;
        class handler_proxy
        {
        public:
            handler_proxy(our_type& aParent) : iParent(aParent), iOrphaned(false)
            {
            }
        public:
            void handle_resolve(const boost::system::error_code& aError, typename resolver_type::iterator aEndPointIterator)
            {
                if (!iOrphaned)
                    iParent.handle_resolve(aError, aEndPointIterator);
            }
            void handle_connect(const boost::system::error_code& aError)
            {
                if (!iOrphaned)
                    iParent.handle_connect(aError);
            }
            void handle_handshake(const boost::system::error_code& aError)
            {
                if (!iOrphaned)
                    iParent.handle_handshake(aError);
            }
            void handle_write(const boost::system::error_code& aError, size_t aBytesTransferred)
            {
                if (!iOrphaned)
                    iParent.handle_write(aError, aBytesTransferred);
            }
            void handle_read(const boost::system::error_code& aError, size_t aBytesTransferred)
            {
                if (!iOrphaned)
                    iParent.handle_read(aError, aBytesTransferred);
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
        struct already_open : std::logic_error { already_open() : std::logic_error("neolib::packet_connection::already_open") {} };
        struct no_socket : std::logic_error { no_socket() : std::logic_error("neolib::packet_connection::no_socket") {} };
        
        // construction
    public:
        basic_packet_connection(
            i_async_task& aIoTask,
            owner_type& aOwner,
            bool aSecure = false,
            protocol_family aProtocolFamily = IPv4) :
            iIoTask(aIoTask), 
            iOwner(aOwner),
            iHandlerProxy(new handler_proxy(*this)),
            iLocalHostName(),
            iLocalPort(0),
            iRemoteHostName(),
            iRemotePort(0),
            iSecure(aSecure),
            iProtocolFamily(aProtocolFamily),
            iError(false),
            iResolver(aIoTask.io_service().native_object<boost::asio::io_service>()),
            iConnected(false),
            iPacketBeingSent(nullptr),
            iReceiveBufferPtr(&iReceiveBuffer[0]),
            iReceivePacket(aOwner.handle_create_empty_packet())
        {
        }
        basic_packet_connection(
            i_async_task& aIoTask,
            owner_type& aOwner, 
            const std::string& aRemoteHostName, 
            unsigned short aRemotePort,
            bool aSecure = false,
            protocol_family aProtocolFamily = IPv4) : 
            iIoTask(aIoTask), 
            iOwner(aOwner),
            iHandlerProxy(new handler_proxy(*this)),
            iLocalHostName(),
            iLocalPort(0),
            iRemoteHostName(aRemoteHostName),
            iRemotePort(aRemotePort),
            iSecure(aSecure),
            iProtocolFamily(aProtocolFamily),
            iError(false),
            iResolver(aIoTask.io_service().native_object<boost::asio::io_service>()),
            iConnected(false),
            iPacketBeingSent(nullptr),
            iReceiveBufferPtr(&iReceiveBuffer[0]),
            iReceivePacket(aOwner.handle_create_empty_packet())
        {
            open();
        }
        ~basic_packet_connection()
        {
            close();
        }
        
        // operations
    public:
        bool open(const std::string& aRemoteHostName, unsigned short aRemotePort, bool aSecure = false, protocol_family aProtocolFamily = IPv4)
        {
            if (opened())
                throw already_open();
            iLocalHostName = std::string();
            iLocalPort = 0;
            iRemoteHostName = aRemoteHostName;
            iRemotePort = aRemotePort;
            iSecure = aSecure;
            iProtocolFamily = aProtocolFamily;
            iError = false;
            return open();
        }
        bool open(bool aAcceptingSocket = false)
        {
            if (opened())
                throw already_open();
            if (!iSecure)
            {
                iSocketHolder = socket_pointer(new socket_type(iIoTask.io_service().native_object<boost::asio::io_service>()));
            }
            else
            {
                if (iSecureStreamContext == nullptr)
                    iSecureStreamContext.reset(new secure_stream_context(boost::asio::ssl::context::sslv23));
                iSocketHolder = secure_stream_pointer(new secure_stream_type(iIoTask.io_service().native_object<boost::asio::io_service>(), *iSecureStreamContext));
            }
            if (aAcceptingSocket)
                return true;
            boost::system::error_code ec;
            socket().open(to_protocol<protocol_type>(iProtocolFamily), ec);
            if (!ec && bind())
            {
                resolve();
                return true;
            }
            return false;
        }
        void close()
        {
            iHandlerProxy->orphan();
            iResolver.cancel();
            if (!std::holds_alternative<std::monostate>(iSocketHolder))
                socket().close();
            iSocketHolder = none;
            bool wasConnected = iConnected;
            iConnected = false;
            iReceiveBufferPtr = &iReceiveBuffer[0];
            if (wasConnected)
                iOwner.handle_connection_closed();
        }
        void send_packet(const packet_type& aPacket, bool aHighPriority = false)
        {
            iSendQueue.insert(aHighPriority ? iSendQueue.begin() : iSendQueue.end(), &aPacket);
            send_any();
        }
        bool opened() const
        {
            if (!iSecure)
            {
                return std::holds_alternative<socket_pointer>(iSocketHolder) && std::get<socket_pointer>(iSocketHolder) != nullptr;
            }
            else
            {
                return std::holds_alternative<secure_stream_pointer>(iSocketHolder) && std::get<secure_stream_pointer>(iSocketHolder) != nullptr;
            }
        }
        bool closed() const
        {
            return !opened();
        }
        bool connected() const
        {
            return iConnected;
        }
        unsigned short local_port() const
        {
            return iLocalPort;
        }
        unsigned short remote_port() const
        {
            return iRemotePort;
        }
        bool has_error() const
        {
            return iError;
        }
        const boost::system::error_code& error() const
        {
            return iErrorCode;
        }
        const endpoint_type& local_end_point() const
        {
            return iLocalEndPoint;
        }
        const endpoint_type& remote_end_point() const
        {
            return iRemoteEndPoint;
        }
        template <typename CharType2, size_t ReceiveBufferSize2>
        friend const typename basic_packet_connection<CharType2, Protocol, ReceiveBufferSize2>::socket_type& do_socket(const basic_packet_connection<CharType2, Protocol, ReceiveBufferSize2>&);
        const socket_type& socket() const
        {
            return do_socket(*this);
        }
        template <typename CharType2, size_t ReceiveBufferSize2>
        friend typename basic_packet_connection<CharType2, Protocol, ReceiveBufferSize2>::socket_type& do_socket(basic_packet_connection<CharType2, Protocol, ReceiveBufferSize2>&);
        socket_type& socket()
        {
            return do_socket(*this);
        }
        const secure_stream_type& secure_stream() const
        {
            if (closed())
                throw no_socket();
            return *std::get<secure_stream_pointer>(iSocketHolder);
        }
        secure_stream_type& secure_stream()
        {
            if (closed())
                throw no_socket();
            return *std::get<secure_stream_pointer>(iSocketHolder);
        }
        void server_accept()
        {
            iConnected = true;
            iLocalHostName = socket().local_endpoint().address().to_string();
            iLocalPort = socket().local_endpoint().port();
            send_any();
            receive_any();
        }
        
        // implementation
    private:
        bool bind()
        {    
            boost::system::error_code ec;
            if (iLocalHostName.empty())
                socket().bind(endpoint_type(to_protocol<protocol_type>(iProtocolFamily), iLocalPort), ec);
            else
            {
                typename resolver_type::iterator result = iResolver.resolve(typename resolver_type::query(iLocalHostName, uint32_to_string<char>(iLocalPort)), ec);
                if (!ec)
                {
                    bool foundGoodMatch = false;
                    for (typename resolver_type::iterator i = result; !foundGoodMatch && i != typename resolver_type::iterator(); ++i)
                    {
                        endpoint_type endpoint = *i;
                        if (to_protocol_family(endpoint.protocol()) & iProtocolFamily)
                        {
                            iLocalEndPoint = endpoint;
                            socket().bind(iLocalEndPoint, ec);
                            foundGoodMatch = true;
                        }
                    }
                    if (!foundGoodMatch)
                    {
                        iLocalEndPoint = *result;
                        socket().bind(iLocalEndPoint, ec);
                    }
                }
            }
            if (!ec)
            {
                iLocalEndPoint = socket().local_endpoint();
                iLocalHostName = socket().local_endpoint().address().to_string();
                iLocalPort = socket().local_endpoint().port();
                return true;
            }
            else
            {
                iError = true;
                iErrorCode = ec;
                iOwner.handle_connection_failure(ec);
                return false;
            }                
        }
        void resolve()
        {
            if (!iRemoteHostName.empty())
            {
                iResolver.async_resolve(typename resolver_type::query(iRemoteHostName, uint32_to_string<char>(iRemotePort)),
                    boost::bind(&handler_proxy::handle_resolve, iHandlerProxy, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
            }
        }

        // implementation
    private:
        void handle_resolve(const boost::system::error_code& aError, typename resolver_type::iterator aEndPointIterator)
        {
            if (closed())
                return;
            if (!aError)
            {
                bool foundGoodMatch = false;
                for (typename resolver_type::iterator i = aEndPointIterator; !foundGoodMatch && i != typename resolver_type::iterator(); ++i)
                {
                    endpoint_type endpoint = *i;
                    if (to_protocol_family(endpoint.protocol()) & iProtocolFamily)
                    {
                        foundGoodMatch = true;
                        iRemoteEndPoint = endpoint;
                        socket().async_connect(iRemoteEndPoint, boost::bind(&handler_proxy::handle_connect, iHandlerProxy, boost::asio::placeholders::error));
                    }
                }
                if (!foundGoodMatch)
                {
                    iRemoteEndPoint = *aEndPointIterator;
                    socket().async_connect(iRemoteEndPoint, boost::bind(&handler_proxy::handle_connect, iHandlerProxy, boost::asio::placeholders::error));
                }
            }
            else
            {
                iError = true;
                iErrorCode = aError;
                iOwner.handle_connection_failure(aError);
            }
        }
        void handle_connect(const boost::system::error_code& aError)
        {
            if (closed())
                return;
            if (!aError)
            {
                iConnected = true;
                if (!iSecure)
                {
                    destroyed_flag destroyed{ *this };
                    iOwner.handle_connection_established();
                    if (destroyed)
                        return;
                    send_any();
                    receive_any();
                }
                else
                {
                      std::get<secure_stream_pointer>(iSocketHolder)->async_handshake(
                          boost::asio::ssl::stream_base::client, 
                          boost::bind(&handler_proxy::handle_handshake, iHandlerProxy, boost::asio::placeholders::error));
                }
            }
            else
            {
                iError = true;
                iErrorCode = aError;
                iOwner.handle_connection_failure(aError);
            }
        }
        void handle_handshake(const boost::system::error_code& aError)
        {
            if (closed())
                return;
            if (!aError)
            {
                destroyed_flag destroyed{ *this };
                iOwner.handle_connection_established();
                if (destroyed)
                    return;
                send_any();
                receive_any();
            }
            else
            {
                iError = true;
                iErrorCode = aError;
                iOwner.handle_connection_failure(aError);
            }
        }            
        void send_any()
        {
            if (!connected())
                return;
            if (iSendQueue.empty())
                return;
            if (iPacketBeingSent != nullptr)
                return;
            iPacketBeingSent = iSendQueue.front();
            iSendQueue.pop_front();
            if (!iSecure)
            {
                boost::asio::async_write(
                    socket(), 
                    boost::asio::buffer(iPacketBeingSent->data(), iPacketBeingSent->length()),
                    boost::bind(
                        &handler_proxy::handle_write, 
                        iHandlerProxy,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
            }
            else
            {
                boost::asio::async_write(
                    secure_stream(), 
                    boost::asio::buffer(iPacketBeingSent->data(), iPacketBeingSent->length()),
                    boost::bind(
                        &handler_proxy::handle_write, 
                        iHandlerProxy,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
            }
        }
        void receive_any()
        {
            if (!connected())
                return;
            
            if (!iSecure)
            {
                socket().async_read_some(
                    boost::asio::buffer(iReceiveBufferPtr, iReceiveBuffer.size() - (iReceiveBufferPtr - &iReceiveBuffer[0])),
                    boost::bind(
                    &handler_proxy::handle_read, 
                    iHandlerProxy,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            }
            else
            {
                secure_stream().async_read_some(
                    boost::asio::buffer(iReceiveBufferPtr, iReceiveBuffer.size() - (iReceiveBufferPtr - &iReceiveBuffer[0])),
                    boost::bind(
                    &handler_proxy::handle_read, 
                    iHandlerProxy,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
            }
        }
        void handle_write(const boost::system::error_code& aError, size_t)
        {
            destroyed_flag destroyed{ *this };
            if (closed())
                return;
            const_packet_pointer sentPacket = iPacketBeingSent;
            iPacketBeingSent = nullptr;
            if (!aError)
            {
                iOwner.handle_packet_sent(*sentPacket);
                if (destroyed)
                    return;
                send_any();
            }
            else
            {
                iError = true;
                iErrorCode = aError;
                iOwner.handle_transfer_failure(*sentPacket, aError);
                if (destroyed)
                    return;
                close();
            }
        }
        void handle_read(const boost::system::error_code& aError, size_t aBytesTransferred)
        {
            destroyed_flag destroyed{ *this };
            if (closed())
                return;
            if (!aError)
            {
                typename packet_type::const_pointer start = reinterpret_cast<typename packet_type::pointer>(&iReceiveBuffer[0]);
                typename packet_type::const_pointer end = start + aBytesTransferred / sizeof(CharType);
                while(iReceivePacket->take_some(start, end))
                {
                    if (!iReceivePacket->empty())
                    {
                        iOwner.handle_packet_arrived(*iReceivePacket);
                        if (destroyed)
                            return;
                        iReceivePacket->clear();
                    }
                }
                iReceiveBufferPtr = std::copy(
                    reinterpret_cast<typename receive_buffer::const_pointer>(start), 
                    reinterpret_cast<typename receive_buffer::const_pointer>(iReceiveBufferPtr + aBytesTransferred),
                    &iReceiveBuffer[0]);
                receive_any();
            }
            else
            {
                if (aError.value() != boost::asio::error::eof && opened())
                {
                    iError = true;
                    iErrorCode = aError;
                    iReceivePacket->clear();
                    iOwner.handle_transfer_failure(*iReceivePacket, aError);
                    if (destroyed)
                        return;
                }
                close();  
            }
        }
        
        // attibutes
    private:
        i_async_task& iIoTask;
        owner_type& iOwner;
        std::shared_ptr<handler_proxy> iHandlerProxy;
        std::string iLocalHostName;
        unsigned short iLocalPort;
        std::string iRemoteHostName;
        unsigned short iRemotePort;
        bool iSecure;
        protocol_family iProtocolFamily;
        bool iError;
        boost::system::error_code iErrorCode;
        resolver_type iResolver;
        endpoint_type iLocalEndPoint;
        endpoint_type iRemoteEndPoint;
        secure_stream_context_pointer iSecureStreamContext;
        socket_holder_type iSocketHolder;
        bool iConnected;
        send_queue iSendQueue;
        const_packet_pointer iPacketBeingSent;
        receive_buffer iReceiveBuffer;
        typename receive_buffer::pointer iReceiveBufferPtr;
        typename packet_type::clone_pointer iReceivePacket;
    };

    template <typename CharType, size_t ReceiveBufferSize>
    inline const typename basic_packet_connection<CharType, tcp_protocol, ReceiveBufferSize>::socket_type& do_socket(const basic_packet_connection<CharType, tcp_protocol, ReceiveBufferSize>& aConnection)
    {
        typedef basic_packet_connection<CharType, tcp_protocol, ReceiveBufferSize> connection_type;
        if (aConnection.closed())
            throw typename connection_type::no_socket();
        if (!aConnection.iSecure)
        {
            return *std::get<typename connection_type::socket_pointer>(aConnection.iSocketHolder);
        }
        else
        {
            return static_cast<const typename connection_type::socket_type&>(std::get<typename connection_type::secure_stream_pointer>(aConnection.iSocketHolder)->lowest_layer());
        }
    }

    template <typename CharType, size_t ReceiveBufferSize>
    inline typename basic_packet_connection<CharType, tcp_protocol, ReceiveBufferSize>::socket_type& do_socket(basic_packet_connection<CharType, tcp_protocol, ReceiveBufferSize>& aConnection) 
    {
        return const_cast<typename basic_packet_connection<CharType, tcp_protocol, ReceiveBufferSize>::socket_type&>(do_socket(const_cast<const basic_packet_connection<CharType, tcp_protocol, ReceiveBufferSize>&>(aConnection)));
    }

    template <typename CharType, size_t ReceiveBufferSize>
    inline const typename basic_packet_connection<CharType, udp_protocol, ReceiveBufferSize>::socket_type& do_socket(const basic_packet_connection<CharType, udp_protocol, ReceiveBufferSize>& aConnection)
    {
        typedef basic_packet_connection<CharType, udp_protocol, ReceiveBufferSize> connection_type;
        if (aConnection.closed())
            throw typename connection_type::no_socket();
        return *std::get<typename connection_type::socket_pointer>(aConnection.iSocketHolder);
    }

    template <typename CharType, size_t ReceiveBufferSize>
    inline typename basic_packet_connection<CharType, udp_protocol, ReceiveBufferSize>::socket_type& do_socket(basic_packet_connection<CharType, udp_protocol, ReceiveBufferSize>& aConnection) 
    {
        return const_cast<typename basic_packet_connection<CharType, udp_protocol, ReceiveBufferSize>::socket_type&>(do_socket(const_cast<const basic_packet_connection<CharType, udp_protocol, ReceiveBufferSize>&>(aConnection)));
    }
}
