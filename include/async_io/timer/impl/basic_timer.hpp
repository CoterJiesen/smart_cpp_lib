#ifndef __TIMER_BASIC_TIMER_HPP
#define __TIMER_BASIC_TIMER_HPP

#include "timer_service.hpp"


namespace async { namespace timer {

	// ---------------------------------------
	// class basic_timer_t

	template < typename ServiceT >
	class basic_timer_t
	{
		typedef ServiceT timer_service_t;

	private:
		timer_service_t &service_;		// service
		std::uint32_t id_;

	public:
		explicit basic_timer_t(timer_service_t &service)
			: service_(service)
			, id_(0)
		{}

		// ���ܻص���������ע��һ��Timer
		template < typename HandlerT >
		basic_timer_t(timer_service_t &service, long period, long due, HandlerT &&handler)
			: service_(service)
			, id_(0)
		{
			id_ = service_.add_timer(period, due, std::forward<HandlerT>(handler));
		}
		~basic_timer_t()
		{
			if( id_ != 0 )
				service_.erase_timer(id_);
		}

	private:
		basic_timer_t(const basic_timer_t &);
		basic_timer_t &operator=(const basic_timer_t &);

	public:
		// ����ʱ����
		// period ʱ����
		// delay �ӳ�ʱ��
		void set_timer(long period, long delay = 0)
		{
			assert(id_ != 0);
			service_.set_timer(id_, period, delay);
		}

		// ȡ��Timer
		void cancel()
		{
			assert(id_ != 0);
			service_.erase_timer(id_);
		}

		// �첽�ȴ�
		void async_wait()
		{
			assert(id_ != 0);
			service_.async_wait(id_);
		}

		template < typename HandlerT >
		void async_wait(HandlerT &&handler, long period, long delay = 0)
		{
			if( id_ == 0 )
				id_ = service_.add_timer(period, delay, handler);

			async_wait();
		}
	};
}
}



#endif