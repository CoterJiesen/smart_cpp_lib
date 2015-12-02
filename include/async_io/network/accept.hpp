#ifndef __NETWORK_ACCEPT_HPP
#define __NETWORK_ACCEPT_HPP


#include <system_error>
#include <cstdint>
#include <functional>

#include "../basic.hpp"
#include "socket.hpp"

namespace async {
	namespace service
	{
		extern std::_Ph<2> _Socket;
		extern std::_Ph<3> _Address;
	}

	namespace network { namespace details {

		static const std::uint32_t SOCKET_ADDR_SIZE = sizeof(sockaddr_in) + 16;

		// Hook User Accept Callback
		template < typename HandlerT >
		struct accept_handle_t
		{	
			socket_handle_t &acceptor_;
			std::shared_ptr<socket_handle_t> remote_sck_;
			HandlerT handler_;

			char address_buffer_[2 * SOCKET_ADDR_SIZE];

			accept_handle_t(socket_handle_t &acceptor, std::shared_ptr<socket_handle_t> &&remoteSocket, HandlerT &&handler)
				: acceptor_(acceptor)
				, remote_sck_(std::move(remoteSocket))
				, handler_(std::move(handler))
			{}

			accept_handle_t(accept_handle_t &&rhs)
				: acceptor_(rhs.acceptor_)
				, remote_sck_(std::move(rhs.remote_sck_))
				, handler_(std::move(rhs.handler_))
			{}

			~accept_handle_t()
			{

			}

		private:
			accept_handle_t(const accept_handle_t &);
			accept_handle_t &operator=(const accept_handle_t &);

		public:
			void operator()(const std::error_code &error, std::uint32_t size)
			{
				// ����Listen socket����
				update_accept_context context((std::uint32_t)acceptor_.native_handle());
				remote_sck_->set_option(context);

				ip_address addr = *(std::uint32_t *)(address_buffer_ + SOCKET_ADDR_SIZE + 4);
				handler_(std::cref(error), std::ref(remote_sck_), std::cref(addr));
			}
		};
	}

	}

}



#endif