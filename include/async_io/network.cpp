#include "network.hpp"

#include <sstream>
#include <memory>

#include "../win32/network/network_helper.hpp"
#include "../utility/move_wrapper.hpp"
#include "../memory_pool/sgi_memory_pool.hpp"
#include "../memory_pool/fixed_memory_pool.hpp"
#include "../extend_stl/allocator/container_allocator.hpp"
#include "../utility/object_pool.hpp"
#include "../extend_stl/container/sync_container.hpp"

namespace async { namespace network {

	typedef std::shared_ptr<socket_handle_t> socket_ptr;
	typedef memory_pool::mt_memory_pool socket_memory_pool_t;
	typedef stdex::allocator::pool_allocator_t<socket_ptr, socket_memory_pool_t> socket_allocator_t;
	typedef stdex::container::sync_sequence_container_t<socket_ptr> socket_pool_list_t;

}
}

namespace utility {

	template < >
	struct object_pool_traits_t<async::network::socket_pool_list_t>
	{
		typedef async::network::socket_pool_list_t pool_t;
		typedef async::network::socket_ptr value_t;

		static value_t pop(pool_t &l)
		{
			if( l.empty() )
			{
				return value_t();
			}
			else
			{
				value_t val = l.pop_back();
				return val;
			}
		}

		static void push(pool_t &l, value_t && val)
		{
			l.push_back(std::move(val));
		}
	};
}


namespace async { namespace network {

	
	std::string error_msg(const std::error_code &err)
	{
		std::ostringstream oss;
		char *buffer = 0;

		DWORD ret = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			0, err.value(), 0, (LPSTR)&buffer, 0, 0);
		if( ret == 0 )
			return std::string();

		oss << "Win32 Error(" << err << ") : " << buffer;

		::LocalFree(buffer);
		return std::move(oss.str());
	}

	template < typename PoolT >
	session_ptr create_session(server &svr,
							   PoolT &pool,
							   std::shared_ptr<socket_handle_t> &&sck,
							   const error_handler_type &error_handler,
							   const disconnect_handler_type &disconnect_handler)
	{
		typedef stdex::allocator::pool_allocator_t<session, PoolT> pool_allocator_t;

		return std::allocate_shared<session>(
			pool_allocator_t(pool),
			svr,
			std::move(sck),
			error_handler,
			disconnect_handler);
	}


	struct server::impl
	{
		server &svr_;
		service::io_dispatcher_t io_;
		tcp::accpetor acceptor_;

		error_handler_type error_handle_;
		accept_handler_type accept_handle_;
		disconnect_handler_type disconnect_handle_;

		typedef memory_pool::sgi_memory_pool_t<true, 256> session_pool_t;
		typedef memory_pool::sgi_memory_pool_t<true, 256> session_allocator_pool_t;

		typedef utility::object_pool_t<socket_handle_t, socket_pool_list_t> socket_pool_t;

		session_pool_t session_pool_;
		session_allocator_pool_t session_allocator_pool_;
		socket_pool_t socket_pool_;

		std::unique_ptr<std::thread> thread_;

		impl(server &svr, std::uint16_t port, std::uint32_t thr_cnt)
			: svr_(svr)
			, io_([this](const std::string &msg){ error_handle_(nullptr, msg); })
			, acceptor_(io_, tcp::v4(), port, INADDR_ANY, false)
			, socket_pool_([this]()
		{
			typedef stdex::allocator::pool_allocator_t<session, session_pool_t> pool_allocator_t;

			tcp v4_ver = tcp::v4();
			auto sck = std::allocate_shared<socket_handle_t>(pool_allocator_t(), io_, v4_ver.family(), v4_ver.type(), v4_ver.protocol());

			sck->set_option(network::linger(true, 0));
			sck->set_option(network::no_delay(true));
			
			return sck;
		})
		{
		}

		void _handle_accept(const std::error_code &error, std::shared_ptr<socket_handle_t> &remote_sck, const ip_address &addr)
		{
			auto val = create_session(svr_, session_pool_, std::move(remote_sck), error_handle_, disconnect_handle_);

			if( error )
			{
				auto msg = error_msg(error);
				error_handle_(val, msg);
				return;
			}

			if( accept_handle_ != nullptr )
			{
				accept_handle_(std::cref(val), std::cref(addr));
			}
		}

		void _thread_impl()
		{
			static const std::uint32_t MAX_ACCEPT_NUM = 20;

			// ͨ��ʹ��WSAEventSelect���ж��Ƿ����㹻��AcceptEx�����߼���һ���������Ŀͻ�����
			HANDLE accept_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			::WSAEventSelect(acceptor_.native_handle(), accept_event, FD_ACCEPT);
			tcp v4_ver = tcp::v4();

			typedef stdex::allocator::pool_allocator_t<char, session_allocator_pool_t> pool_allocator_t;
			pool_allocator_t pool_allocator(session_allocator_pool_);

			while( true )
			{
				DWORD ret = ::WaitForSingleObjectEx(accept_event, INFINITE, TRUE);
				if( ret == WAIT_FAILED || ret == WAIT_IO_COMPLETION )
					break;
				else if( ret != WAIT_OBJECT_0 )
					continue;

				// Ͷ�ݽ�������
				for( std::uint32_t i = 0; i != MAX_ACCEPT_NUM; ++i )
				{
					auto sck = socket_pool_.raw_aciquire();

					try
					{
						acceptor_.async_accept(std::move(sck),
											   std::bind(&impl::_handle_accept, this, service::_Error, service::_Socket, service::_Address),
											   pool_allocator);
					}
					catch( std::exception &e)
					{
						e;
						--i;
					}
				}

			}

			::CloseHandle(accept_event);
		}
	};

	session::session(server &svr, std::shared_ptr<socket_handle_t> &&sck, 
		const error_handler_type &error_handler, const disconnect_handler_type &disconnect_handler)
		: svr_(svr)
		, sck_(std::move(sck))
		, data_(nullptr)
		, is_valid_(true)
		, error_handler_(error_handler)
		, disconnect_handler_(disconnect_handler)
	{
		
	}
	session::~session()
	{
		svr_.impl_->socket_pool_.raw_release(std::move(sck_));
	}

	std::string session::get_ip() const
	{
		if( !sck_ )
			return "socket was closed";

		if( !sck_->is_open() )
			return "unknown ip, socket was closed";

		return win32::network::ip_2_string(win32::network::get_sck_ip(sck_->native_handle()));
	}


	void session::shutdown()
	{
		sck_->cancel();
	}

	void session::disconnect()
	{
		is_valid_ = false;
		auto this_val = shared_from_this();

		try
		{
			typedef decltype(svr_.impl_->session_allocator_pool_) pool_t;
			typedef stdex::allocator::pool_allocator_t<char, pool_t> pool_allocator_t;

			sck_->async_disconnect(true, [this, this_val](const std::error_code &e, std::uint32_t sz) mutable
			{
				if( disconnect_handler_ )
					disconnect_handler_(shared_from_this());

			}, pool_allocator_t(svr_.impl_->session_allocator_pool_));
		}
		catch( std::exception &e )
		{
			error_handler_(this_val, e.what());
		}
	}

	
	server::server(std::uint16_t port, std::uint32_t thr_cnt)
		: impl_(std::make_unique<impl>(*this, port, thr_cnt == 0 ? 1 : thr_cnt))
	{

	}

	server::~server()
	{
	}

	bool server::start()
	{
		impl_->thread_ = std::make_unique<std::thread>(std::bind(&impl::_thread_impl, impl_.get()));
		return true;
	}


	bool server::stop()
	{
		if( impl_->thread_ )
		{
			::QueueUserAPC([](ULONG_PTR){}, impl_->thread_->native_handle(), 0);
			impl_->thread_->join();
			impl_->thread_.reset();
		}

		impl_->io_.stop();
		impl_->acceptor_.close();

		impl_->error_handle_	= nullptr;
		impl_->accept_handle_	= nullptr;

		return true;
	}

	void server::register_error_handler(const error_handler_type &handler)
	{
		impl_->error_handle_ = handler;
	}

	void server::register_accept_handler(const accept_handler_type &handler)
	{
		impl_->accept_handle_ = handler;
	}

	void server::register_disconnect_handler(const disconnect_handler_type &handler)
	{
		impl_->disconnect_handle_ = handler;
	}

	void server::post(std::function<void(const std::error_code &, std::uint32_t)> &&handler)
	{
		typedef impl::session_allocator_pool_t pool_t;
		typedef stdex::allocator::pool_allocator_t<char, pool_t> pool_allocator_t;

		impl_->io_.post(std::move(handler), pool_allocator_t(impl_->session_allocator_pool_));
	}

	// ----------------------------------

	client::client(service::io_dispatcher_t &io)
		: io_(io)
		, socket_(io_, network::tcp::v4())
	{

	}

	client::~client()
	{
	}

	void client::register_error_handler(const error_handler_type &handler)
	{
		error_handle_ = handler;
	}

	void client::register_connect_handler(const connect_handler_type &handler)
	{
		connect_handle_ = handler;
	}

	void client::register_disconnect_handler(const disconnect_handler_type &handler)
	{
		disconnect_handle_ = handler;
	}


	bool client::start(const std::string &ip, std::uint16_t port)
	{
		try
		{
			if( !socket_.is_open() )
			{
				socket_.open();
			}

			socket_.connect(port, network::ip_address::parse(ip));
			_on_connect(std::make_error_code((std::errc)::GetLastError()));
		}
		catch(exception::exception_base &e)
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::string(e.what()));
			return false;
		}
		catch(std::exception &e)
		{
			if( error_handle_ )
				error_handle_(std::string(e.what()));
			return false;
		}

		return true;
	}

	bool client::async_start(const std::string &ip, std::uint16_t port)
	{
		try
		{
			if( !socket_.is_open() )
			{
				socket_.open();
			}

			socket_.async_connect(network::ip_address::parse(ip), port, connect_handle_, std::allocator<char>());
		}
		catch( exception::exception_base &e )
		{
			e.dump();

			if( error_handle_ )
				error_handle_(std::string(e.what()));
			return false;
		}
		catch( std::exception &e )
		{
			if( error_handle_ )
				error_handle_(std::string(e.what()));
			return false;
		}

		return true;
	}

	void client::stop()
	{
		if( socket_.is_open() )
			socket_.close();
	}

	// read & write no exception safe

	bool client::send(const char *buf, std::uint32_t len)
	{
		return _run_impl([&]()
		{
			service::write(socket_, service::buffer(buf, len));
		});
	}

	bool client::recv(char *buf, std::uint32_t len)
	{
		return _run_impl([&]()
		{
			service::read(socket_, service::buffer(buf, len));
		});
	}

	std::uint32_t client::read_some(char *buf, std::uint32_t len)
	{
		std::uint32_t size = 0;
		_run_impl([&]()
		{
			size = service::read(socket_, service::buffer(buf, len), service::transfer_at_least(1),
				[](const std::error_code &e, std::uint32_t size)
			{});
		});

		return size;
	}

	void client::disconnect()
	{
		try
		{
			stop();

			if( disconnect_handle_ )
				disconnect_handle_();
		}
		catch(...)
		{
			assert(0 && "has an exception in disconnect_handler_");
		}
	}

	void client::_on_connect(const std::error_code &error)
	{
		try
		{
			if( connect_handle_ )
				connect_handle_(error.value() == 0);

			if( !error )
			{
				socket_.set_option(network::no_delay(true));
				socket_.set_option(network::linger(true, 0));
			}
			else
			{
				if( error_handle_ )
					error_handle_(error_msg(error));
				disconnect();
			}
		}
		catch(...)
		{
			assert(0 && "has an exception in connect_handler_");
		}
	}

	

}
}