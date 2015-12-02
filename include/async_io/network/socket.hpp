#ifndef __ASYNC_NETWORK_SOCKET_HPP
#define __ASYNC_NETWORK_SOCKET_HPP

#include "../service/dispatcher.hpp"
#include "../service/read_write_buffer.hpp"
#include "../service/multi_buffer.hpp"

#include "ip_address.hpp"
#include "socket_provider.hpp"


namespace async { namespace network {
	// forward declare

	class socket_handle_t;
	typedef std::unique_ptr<socket_handle_t> socket_handle_ptr;

	//----------------------------------------------------------------------
	// class Socket

	class socket_handle_t
	{
	public:
		typedef service::io_dispatcher_t	dispatcher_type;
		typedef SOCKET native_handle_type;

	private:
		// socket handle
		native_handle_type socket_;

		// IO����
		dispatcher_type &io_;

	public:
		explicit socket_handle_t(dispatcher_type &);
		socket_handle_t(dispatcher_type &, native_handle_type sock);
		socket_handle_t(dispatcher_type &, int family, int type, int protocol);

		socket_handle_t(socket_handle_t &&);
		socket_handle_t &operator=(socket_handle_t &&);

		~socket_handle_t();

	private:
		socket_handle_t(const socket_handle_t &);
		socket_handle_t &operator=(const socket_handle_t &);

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
		void bind(int family, std::uint16_t uPort, const ip_address &addr);
		// listen
		void listen(int nMax);

		dispatcher_type &get_dispatcher()
		{
			return io_;
		}

		// �������ûص��ӿ�,ͬ������
	public:
		socket_handle_ptr accept();
		void connect(int family, const ip_address &addr, std::uint16_t uPort);
		void dis_connect(int shut, bool bReuseSocket = true);

		std::uint32_t read(service::mutable_buffer_t &buffer, DWORD flag = 0);
		std::uint32_t write(const service::const_buffer_t &buffer, DWORD flag = 0);

		std::uint32_t send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, DWORD flag);
		std::uint32_t recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, DWORD flag);

		// �첽���ýӿ�
	public:
		// szOutSizeָ������Ļ�������С��������AcceptԶ�����Ӻ����յ���һ�����ݰ��ŷ���
		template < typename HandlerT, typename AllocatorT >
		void async_accept(std::shared_ptr<socket_handle_t> &&remote_sck, HandlerT &&callback, const AllocatorT &allocator);
		// �첽������Ҫ�Ȱ󶨶˿�
		template < typename HandlerT, typename AllocatorT >
		void async_connect(const ip_address &addr, std::uint16_t uPort, HandlerT &&callback, const AllocatorT &allocator);

		// �첽�Ͽ�����
		template < typename HandlerT, typename AllocatorT >
		void async_disconnect(bool is_reuse, HandlerT &&callback, const AllocatorT &allocator);

		// �첽TCP��ȡ
		template < typename HandlerT, typename AllocatorT >
		void async_read(service::mutable_buffer_t &buf, HandlerT &&callback, const AllocatorT &allocator);
		
		// �첽TCPд��
		template < typename HandlerT, typename AllocatorT >
		void async_write(const service::const_buffer_t &buf, HandlerT &&callback, const AllocatorT &allocator);
		template < typename ParamT, typename AllocatorT >
		void async_write(ParamT &&param, const AllocatorT &allocator);

		// �첽UDP��ȡ
		template < typename HandlerT >
		void async_send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, HandlerT &&callback);
		// �첽UDP����
		template < typename HandlerT >
		void async_recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, HandlerT &&callback);

	};
}
}


#include "accept.hpp"
#include "connect.hpp"

namespace async { namespace network {

	// ---------------------------

	inline bool socket_handle_t::is_open() const
	{
		return socket_ != INVALID_SOCKET;
	}

	template< typename IOCtrlT >
	inline bool socket_handle_t::io_control(IOCtrlT &ioCtrl)
	{
		if( !is_open() )
			return false;

		DWORD dwRet = 0;
		if( 0 != ::WSAIoctl(socket_, ioCtrl.cmd(), ioCtrl.in_buffer(), ioCtrl.in_buffer_size(), 
			ioCtrl.out_buffer(), ioCtrl.out_buffer_size(), &dwRet, 0, 0) )
			throw service::win32_exception_t("WSAIoCtl");

		return true;
	}

	template<typename SocketOptionT>
	inline bool socket_handle_t::set_option(const SocketOptionT &option)
	{
		if( !is_open() )
			return false;

		return SOCKET_ERROR != ::setsockopt(socket_, option.level(), option.name(), option.data(), option.size());
	}

	template<typename SocketOptionT>
	inline bool socket_handle_t::get_option(SocketOptionT &option) const
	{
		if( !is_open() )
			return false;

		int sz = option.size();
		if( SOCKET_ERROR != ::getsockopt(socket_, option.level(), option.name(), option.data(), &sz) )
		{
			option.resize(sz);
			return true;
		}
		else
			return false;
	}

	template < typename HandlerT, typename AllocatorT >
	void socket_handle_t::async_accept(std::shared_ptr<socket_handle_t> &&remote_sck, HandlerT &&callback, const AllocatorT &allocator)
	{
		if( !is_open() ) 
			throw service::network_exception("Socket not open");

		native_handle_type sck = *remote_sck;

		typedef details::accept_handle_t<HandlerT> HookAcceptor;
		HookAcceptor accept_hook(*this, std::move(remote_sck), std::forward<HandlerT>(callback));
		auto p = service::make_async_callback(std::move(accept_hook), allocator);
		service::async_callback_base_ptr async_result(p);

		// ����szOutSide��С�жϣ��Ƿ���Ҫ����Զ�̿ͻ�����һ�����ݲŷ��ء�
		// ���Ϊ0�����������ء�������0����������ݺ��ٷ���
		DWORD dwRecvBytes = 0;
		if( !socket_provider::singleton().AcceptEx(socket_, sck, p->handler_.address_buffer_, 0,
			details::SOCKET_ADDR_SIZE, details::SOCKET_ADDR_SIZE, &dwRecvBytes, async_result.get()) 
			&& ::WSAGetLastError() != ERROR_IO_PENDING )
			throw service::win32_exception_t("AcceptEx");

		async_result.release();
	}

	// �첽���ӷ���
	template < typename HandlerT, typename AllocatorT >
	void socket_handle_t::async_connect(const ip_address &addr, u_short uPort, HandlerT &&callback, const AllocatorT &allocator)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		sockaddr_in localAddr		= {0};
		localAddr.sin_family		= AF_INET;

		// �ܱ�̬����Ҫ��bind
		int ret = ::bind(socket_, reinterpret_cast<const sockaddr *>(&localAddr), sizeof(localAddr));
		if( ret != 0 )
			throw service::win32_exception_t("bind");

		sockaddr_in remoteAddr		= {0};
		remoteAddr.sin_family		= AF_INET;
		remoteAddr.sin_port			= ::htons(uPort);
		remoteAddr.sin_addr.s_addr	= ::htonl(addr.address());

		typedef details::connect_handle_t<HandlerT> HookConnect;
		HookConnect connect_hook(*this, std::forward<HandlerT>(callback));
		service::async_callback_base_ptr async_result(service::make_async_callback(std::move(connect_hook), allocator));

		if( !socket_provider::singleton().ConnectEx(socket_, reinterpret_cast<SOCKADDR *>(&remoteAddr), sizeof(SOCKADDR), 0, 0, 0, async_result.get()) 
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("ConnectionEx");

		async_result.release();
	}


	// �첽�ӽ�������
	template < typename HandlerT, typename AllocatorT >
	void socket_handle_t::async_read(service::mutable_buffer_t &buf, HandlerT &&callback, const AllocatorT &allocator)
	{
		WSABUF wsabuf = {0};
		wsabuf.buf = buf.data();
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback), allocator));

		int ret = ::WSARecv(socket_, &wsabuf, 1, &dwSize, &dwFlag, asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSARecv");
		//else if( ret == 0 )
		//	asynResult->invoke(std::error_code(), dwSize);
		else
			asynResult.release();
	}

	// �첽��������
	template < typename HandlerT, typename AllocatorT >
	void socket_handle_t::async_write(const service::const_buffer_t &buf, HandlerT &&callback, const AllocatorT &allocator)
	{
		WSABUF wsabuf = {0};
		wsabuf.buf = const_cast<char *>(buf.data());
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback), allocator));

		int ret = ::WSASend(socket_, &wsabuf, 1, &dwSize, dwFlag, asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASend");
		//else if( ret == 0 )
		//	asynResult->invoke(std::error_code(), dwSize);
		else
			asynResult.release();
	}


	template < typename ParamT, typename AllocatorT >
	void socket_handle_t::async_write(ParamT &&param, const AllocatorT &allocator)
	{
		auto async_callback_val = service::make_async_callback(std::forward<ParamT>(param), allocator);
		service::async_callback_base_ptr asynResult(async_callback_val);

		auto buffers = async_callback_val->handler_.buffers();
		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		int ret = ::WSASend(socket_, buffers.data(), (std::uint32_t)buffers.size(), &dwSize, dwFlag, asynResult.get(), NULL);
		if( 0 != ret
		   && ::WSAGetLastError() != WSA_IO_PENDING )
		   throw service::win32_exception_t("WSASend");
		//else if( ret == 0 )
		//	asynResult->invoke(std::error_code(), dwSize);
		else
			asynResult.release();
	}

	// �첽�ر�����
	template < typename HandlerT, typename AllocatorT >
	void socket_handle_t::async_disconnect(bool is_reuse, HandlerT &&callback, const AllocatorT &allocator)
	{
		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback), allocator));

		DWORD dwFlags = is_reuse ? TF_REUSE_SOCKET : 0;

		if( !socket_provider::singleton().DisconnectEx(socket_, asynResult.get(), dwFlags, 0) 
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("DisConnectionEx");

		asynResult.release();
	}



	// �첽UDPд��
	template < typename HandlerT >
	void socket_handle_t::async_send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, HandlerT &&callback)
	{
		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		WSABUF wsabuf = {0};
		wsabuf.buf = const_cast<char *>(buf.data());
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		int ret = ::WSASendTo(socket_, &wsabuf, 1, &dwSize, dwFlag, reinterpret_cast<const sockaddr *>(addr), addr == 0 ? 0 : sizeof(*addr), asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASendTo");
		//else if( ret == 0 )
		//	asynResult->invoke(std::error_code(), dwSize);
		else
			asynResult.release();
	}	

	// �첽UDP����
	template < typename HandlerT >
	void socket_handle_t::async_recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, HandlerT &&callback)
	{
		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		WSABUF wsabuf = {0};
		wsabuf.buf = buf.data();
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;
		int addrLen = (addr == 0 ? 0 : sizeof(addr));

		int ret = ::WSARecvFrom(socket_, &wsabuf, 1, &dwSize, &dwFlag, reinterpret_cast<sockaddr *>(addr), &addrLen, asynResult.get(), NULL);
		if( 0 != ret
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSARecvFrom");
		//else if( ret == 0 )
		//	asynResult->invoke(std::error_code(), dwSize);
		else
			asynResult.release();
	}
}
}



#endif