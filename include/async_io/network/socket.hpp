#ifndef __NETWORK_SOCKET_HPP
#define __NETWORK_SOCKET_HPP



#include "../IOCP/dispatcher.hpp"
#include "../IOCP/buffer.hpp"
#include "../IOCP/read_write_buffer.hpp"


#include "ip_address.hpp"
#include "socket_provider.hpp"


namespace async
{


	namespace network
	{
		// forward declare

		class socket_handle;
		typedef std::tr1::shared_ptr<socket_handle> socket_handle_ptr;



		//----------------------------------------------------------------------
		// class Socket

		class socket_handle
		{
		public:
			typedef iocp::io_dispatcher	dispatcher_type;
			typedef SOCKET native_handle_type;

		private:
			// socket handle
			native_handle_type socket_;

			// IO����
			dispatcher_type &io_;

		public:
			explicit socket_handle(dispatcher_type &);
			socket_handle(dispatcher_type &, native_handle_type sock);
			socket_handle(dispatcher_type &, int family, int type, int protocol);
			~socket_handle();

			// non-copyable
		private:
			socket_handle(const socket_handle &);
			socket_handle &operator=(const socket_handle &);

		public:
			// explicitת��
			operator native_handle_type &()				{ return socket_; }
			operator const native_handle_type &() const	{ return socket_; }

			// ��ʾ��ȡ
			native_handle_type &native_handle()				{ return socket_; }
			const native_handle_type &native_handle() const	{ return socket_; }

		public:
			// WSAIoctl
			template < typename IOCtrlT >
			bool io_control(IOCtrlT &ioCtrl);

			// setsockopt
			template < typename SocketOptionT >
			bool set_option(const SocketOptionT &option);

			// getsockopt
			template < typename SocketOptionT >
			bool get_option(SocketOptionT &option) const;

			// WSASocket
			void open(int family, int nType, int nProtocol);
			// shutdown
			void shutdown(int shut);
			// closesocket
			void close();

			bool is_open() const;
			// CancelIO/CancelIOEx
			void cancel();

			// bind
			void bind(int family, u_short uPort, const ip_address &addr);
			// listen
			void listen(int nMax);

			dispatcher_type &get_dispatcher()
			{
				return io_;
			}

			// �������ûص��ӿ�,ͬ������
		public:
			socket_handle_ptr accept();
			void connect(int family, const ip_address &addr, u_short uPort);
			void dis_connect(int shut, bool bReuseSocket = true);

			size_t read(iocp::mutable_buffer &buffer, DWORD flag);
			size_t write(const iocp::const_buffer &buffer, DWORD flag);

			size_t send_to(const iocp::const_buffer &buf, const SOCKADDR_IN *addr, DWORD flag);
			size_t recv_from(iocp::mutable_buffer &buf, SOCKADDR_IN *addr, DWORD flag);

			// �첽���ýӿ�
		public:
			// szOutSizeָ������Ļ�������С��������AcceptԶ�����Ӻ����յ���һ�����ݰ��ŷ���
			template < typename HandlerT >
			void async_accept(const socket_handle_ptr &remote_sck, const HandlerT &callback);
			// �첽������Ҫ�Ȱ󶨶˿�
			template < typename HandlerT >
			void async_connect(const ip_address &addr, u_short uPort, const HandlerT &callback);

			// �첽�Ͽ�����
			void async_disconnect(const iocp::rw_callback_type &callback, bool bReuseSocket);

			// �첽TCP��ȡ
			void async_read(iocp::mutable_buffer &buf, const iocp::rw_callback_type &callback);

			// �첽TCPд��
			void async_write(const iocp::const_buffer &buf, const iocp::rw_callback_type &callback);

			// �첽UDP��ȡ
			void async_send_to(const iocp::const_buffer &buf, const SOCKADDR_IN *addr, const iocp::rw_callback_type &callback);

			// �첽UDP����
			void async_recv_from(iocp::mutable_buffer &buf, SOCKADDR_IN *addr, const iocp::rw_callback_type &callback);
		};
	}
}

namespace async
{
	namespace iocp
	{
		typedef async::network::socket_handle Socket;

		// Socket ����
		template<>
		struct object_factory_t< Socket >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(Socket)>		PoolType;
			typedef object_pool_t< PoolType >									ObjectPoolType;
		};
	}
}

#include "accept.hpp"
#include "connect.hpp"

namespace async
{
	namespace network
	{

		inline socket_handle_ptr make_socket(socket_handle::dispatcher_type &io)
		{
			return socket_handle_ptr(iocp::object_allocate<socket_handle>(io), &iocp::object_deallocate<socket_handle>);
		}

		inline socket_handle_ptr make_socket(socket_handle::dispatcher_type &io, SOCKET sock)
		{
			return socket_handle_ptr(iocp::object_allocate<socket_handle>(io, sock), &iocp::object_deallocate<socket_handle>);
		}

		inline socket_handle_ptr make_socket(socket_handle::dispatcher_type &io, int family, int type, int protocol)
		{
			return socket_handle_ptr(iocp::object_allocate<socket_handle>(io, family, type, protocol), &iocp::object_deallocate<socket_handle>);
		}


		// ---------------------------

		inline bool socket_handle::is_open() const
		{
			return socket_ != INVALID_SOCKET;
		}


		template< typename IOCtrlT >
		inline bool socket_handle::io_control(IOCtrlT &ioCtrl)
		{
			if( !is_open() )
				return false;

			DWORD dwRet = 0;
			if( 0 != ::WSAIoctl(socket_, ioCtrl.cmd(), ioCtrl.in_buffer(), ioCtrl.in_buffer_size(), 
				ioCtrl.out_buffer(), ioCtrl.out_buffer_size(), &dwRet, 0, 0) )
				throw iocp::win32_exception("WSAIoCtl");

			return true;
		}

		template<typename SocketOptionT>
		inline bool socket_handle::set_option(const SocketOptionT &option)
		{
			if( !is_open() )
				return false;

			return SOCKET_ERROR != ::setsockopt(socket_, option.level(), option.name(), option.data(), option.size());
		}

		template<typename SocketOptionT>
		inline bool socket_handle::get_option(SocketOptionT &option) const
		{
			if( !is_open() )
				return false;

			size_t sz = option.size();
			if( SOCKET_ERROR != ::getsockopt(socket_, option.level(), option.name(), option.data(), sz) )
			{
				option.resize(sz);
				return true;
			}
			else
				return false;
		}

		template < typename HandelrT >
		void socket_handle::async_accept(const socket_handle_ptr &remote_sck, const HandelrT &callback)
		{
			if( !is_open() ) 
				throw exception::exception_base("Socket not open");

			iocp::auto_buffer_ptr accept_buffer = iocp::make_buffer(detail::SOCKET_ADDR_SIZE * 2);

			typedef detail::accept_handle_t<HandelrT> HookAcceptor;
			iocp::async_callback_base_ptr async_result(iocp::make_async_callback<iocp::async_callback>(HookAcceptor(*this, remote_sck, accept_buffer, callback)));

			// ����szOutSide��С�жϣ��Ƿ���Ҫ����Զ�̿ͻ�����һ�����ݲŷ��ء�
			// ���Ϊ0�����������ء�������0����������ݺ��ٷ���
			DWORD dwRecvBytes = 0;
			if( !socket_provider::singleton().AcceptEx(socket_, remote_sck->socket_, accept_buffer->data(), 0,
				detail::SOCKET_ADDR_SIZE, detail::SOCKET_ADDR_SIZE, &dwRecvBytes, async_result.get()) 
				&& ::WSAGetLastError() != ERROR_IO_PENDING )
				throw iocp::win32_exception("AcceptEx");

			async_result.release();
		}

		// �첽���ӷ���
		template < typename HandelrT >
		void socket_handle::async_connect(const ip_address &addr, u_short uPort, const HandelrT &callback)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			sockaddr_in localAddr		= {0};
			localAddr.sin_family		= AF_INET;

			// �ܱ�̬����Ҫ��bind
			int ret = ::bind(socket_, reinterpret_cast<const sockaddr *>(&localAddr), sizeof(localAddr));
			if( ret != 0 )
				throw iocp::win32_exception("bind");

			sockaddr_in remoteAddr		= {0};
			remoteAddr.sin_family		= AF_INET;
			remoteAddr.sin_port			= ::htons(uPort);
			remoteAddr.sin_addr.s_addr	= ::htonl(addr.address());

			typedef detail::connect_handle_t<HandelrT> HookConnect;
			iocp::async_callback_base_ptr async_result(iocp::make_async_callback<iocp::async_callback>(HookConnect(*this, callback)));

			if( !socket_provider::singleton().ConnectEx(socket_, reinterpret_cast<SOCKADDR *>(&remoteAddr), sizeof(SOCKADDR), 0, 0, 0, async_result.get()) 
				&& ::WSAGetLastError() != WSA_IO_PENDING )
				throw iocp::win32_exception("ConnectionEx");

			async_result.release();
		}
		
		
	}
}



#endif