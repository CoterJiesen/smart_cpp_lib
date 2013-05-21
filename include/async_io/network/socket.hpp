#ifndef __ASYNC_NETWORK_SOCKET_HPP
#define __ASYNC_NETWORK_SOCKET_HPP

#include "../service/dispatcher.hpp"
#include "../service/read_write_buffer.hpp"


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

		size_t read(service::mutable_buffer_t &buffer, DWORD flag);
		size_t write(const service::const_buffer_t &buffer, DWORD flag);

		size_t send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, DWORD flag);
		size_t recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, DWORD flag);

		// �첽���ýӿ�
	public:
		// szOutSizeָ������Ļ�������С��������AcceptԶ�����Ӻ����յ���һ�����ݰ��ŷ���
		template < typename AllocT, typename HandlerT >
		void async_accept(socket_handle_t &&remote_sck, AllocT &alloc, HandlerT &&callback);
		// �첽������Ҫ�Ȱ󶨶˿�
		template < typename HandlerT >
		void async_connect(const ip_address &addr, std::uint16_t uPort, HandlerT &&callback);

		// �첽�Ͽ�����
		template < typename HandlerT >
		void async_disconnect(bool is_reuse, HandlerT &&callback);

		// �첽TCP��ȡ
		template < typename HandlerT >
		void async_read(service::mutable_buffer_t &buf, HandlerT &&callback);
		template < typename HandlerT >
		void async_read(service::mutable_array_buffer_t &buf, HandlerT &&callback);

		// �첽TCPд��
		template < typename HandlerT >
		void async_write(const service::const_buffer_t &buf, HandlerT &&callback);
		template < typename HandlerT >
		void async_write(const service::const_array_buffer_t &buf, HandlerT &&callback);

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

	template < typename AllocT, typename HandlerT >
	void socket_handle_t::async_accept(socket_handle_t &&remote_sck, AllocT &alloc, HandlerT &&callback)
	{
		if( !is_open() ) 
			throw service::network_exception("Socket not open");

		struct socket_addr_size_t
		{
			char arr_[details::SOCKET_ADDR_SIZE * 2];
			socket_addr_size_t()
			{
				std::memset(arr_, 0, _countof(arr_));
			}
		};
		auto accept_buffer = std::allocate_shared<socket_addr_size_t>(alloc);
		
		native_handle_type sck = remote_sck.native_handle();

		typedef details::accept_handle_t<HandlerT, std::shared_ptr<socket_addr_size_t>> HookAcceptor;
		HookAcceptor accept_hook(*this, std::move(remote_sck), accept_buffer, std::forward<HandlerT>(callback));
		service::async_callback_base_ptr async_result(service::make_async_callback(std::move(accept_hook)));

		// ����szOutSide��С�жϣ��Ƿ���Ҫ����Զ�̿ͻ�����һ�����ݲŷ��ء�
		// ���Ϊ0�����������ء�������0����������ݺ��ٷ���
		DWORD dwRecvBytes = 0;
		if( !socket_provider::singleton().AcceptEx(socket_, sck, reinterpret_cast<char *>(accept_buffer.get()), 0,
			details::SOCKET_ADDR_SIZE, details::SOCKET_ADDR_SIZE, &dwRecvBytes, async_result.get()) 
			&& ::WSAGetLastError() != ERROR_IO_PENDING )
			throw service::win32_exception_t("AcceptEx");

		async_result.release();
	}

	// �첽���ӷ���
	template < typename HandlerT >
	void socket_handle_t::async_connect(const ip_address &addr, u_short uPort, HandlerT &&callback)
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

		typedef detail::connect_handle_t<HandlerT> HookConnect;
		HookConnect connect_hook(*this, std::forward<HandlerT>(callback));
		service::async_callback_base_ptr async_result(service::make_async_callback(std::move(connect_hook)));

		if( !socket_provider::singleton().ConnectEx(socket_, reinterpret_cast<SOCKADDR *>(&remoteAddr), sizeof(SOCKADDR), 0, 0, 0, async_result.get()) 
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("ConnectionEx");

		async_result.release();
	}


	// �첽�ӽ�������
	template < typename HandlerT >
	void socket_handle_t::async_read(service::mutable_buffer_t &buf, HandlerT &&callback)
	{
		WSABUF wsabuf = {0};
		wsabuf.buf = buf.data();
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		if( 0 != ::WSARecv(socket_, &wsabuf, 1, &dwSize, &dwFlag, asynResult.get(), NULL)
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSARecv");

		asynResult.release();
	}

	// �첽�ӽ�������
	template < typename HandlerT >
	void socket_handle_t::async_read(service::mutable_array_buffer_t &buf, HandlerT &&callback)
	{
		struct mutable_wsa_buf_t
			: WSABUF
		{
			mutable_wsa_buf_t()
			{}
			mutable_wsa_buf_t(std::uint32_t len, char *buf)
			{
				WSABUF::len = len;
				WSABUF::buf = buf;
			}
		};

		stdex::allocator::stack_storage_t<1024> storage;
		typedef stdex::allocator::stack_allocator_t<mutable_wsa_buf_t, 1024> stack_pool_t;
		stack_pool_t alloc(&storage);
		std::vector<mutable_wsa_buf_t, stack_pool_t> wsabuf(alloc);
		wsabuf.reserve(buf.buffer_count());
		auto pos = buf.buffer_count();
		for(auto iter = buf.buffers_.begin(); iter != buf.buffers_.end(); ++iter)
			wsabuf[--pos] = mutable_wsa_buf_t(iter->size(), iter->data());

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		if( 0 != ::WSARecv(socket_, wsabuf.data(), wsabuf.size(), &dwSize, &dwFlag, asynResult.get(), NULL)
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSARecv");

		asynResult.release();
	}

	// �첽��������
	template < typename HandlerT >
	void socket_handle_t::async_write(const service::const_buffer_t &buf, HandlerT &&callback)
	{
		WSABUF wsabuf = {0};
		wsabuf.buf = const_cast<char *>(buf.data());
		wsabuf.len = buf.size();

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		if( 0 != ::WSASend(socket_, &wsabuf, 1, &dwSize, dwFlag, asynResult.get(), NULL)
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASend");

		asynResult.release();
	}

	template < typename HandlerT >
	void socket_handle_t::async_write(const service::const_array_buffer_t &buf, HandlerT &&callback)
	{
		struct const_wsa_buf_t
			: WSABUF
		{
			const_wsa_buf_t()
			{}
			const_wsa_buf_t(std::uint32_t len, const char *buf)
			{
				WSABUF::len = len;
				WSABUF::buf = const_cast<char *>(buf);
			}
		};

		stdex::allocator::stack_storage_t<1024> storage;
		typedef stdex::allocator::stack_allocator_t<const_wsa_buf_t, 1024> stack_pool_t;
		stack_pool_t alloc(&storage);
		std::vector<const_wsa_buf_t, stack_pool_t> wsabuf(alloc);
		
		auto cnt = buf.buffer_count();
		wsabuf.resize(cnt);
		
		for(auto iter = buf.buffers_.cbegin(); iter != buf.buffers_.cend(); ++iter)
			wsabuf[--cnt] = const_wsa_buf_t(iter->size(), iter->data());

		DWORD dwFlag = 0;
		DWORD dwSize = 0;

		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

		if( 0 != ::WSASend(socket_, wsabuf.data(), wsabuf.size(), &dwSize, dwFlag, asynResult.get(), NULL)
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASend");

		asynResult.release();
	}

	// �첽�ر�����
	template < typename HandlerT >
	void socket_handle_t::async_disconnect(bool is_reuse, HandlerT &&callback)
	{
		service::async_callback_base_ptr asynResult(service::make_async_callback(std::forward<HandlerT>(callback)));

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

		if( 0 != ::WSASendTo(socket_, &wsabuf, 1, &dwSize, dwFlag, reinterpret_cast<const sockaddr *>(addr), addr == 0 ? 0 : sizeof(*addr), asynResult.get(), NULL)
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSASendTo");

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

		if( 0 != ::WSARecvFrom(socket_, &wsabuf, 1, &dwSize, &dwFlag, reinterpret_cast<sockaddr *>(addr), &addrLen, asynResult.get(), NULL)
			&& ::WSAGetLastError() != WSA_IO_PENDING )
			throw service::win32_exception_t("WSARecvFrom");

		asynResult.release();
	}
}
}



#endif