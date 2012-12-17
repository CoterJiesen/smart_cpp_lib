#ifndef __TIMER_BASIC_TIMER_HPP
#define __TIMER_BASIC_TIMER_HPP

#include "../../iocp/dispatcher.hpp"

#include "timer_service.hpp"


namespace async
{
	namespace timer
	{
		namespace impl
		{
		
			// ---------------------------------------
			// class basic_timer_t

			template < typename ImplT, typename ServiceT >
			class basic_timer_t
			{
			public:
				typedef timer_service_t<ImplT, ServiceT>			timer_service;
				typedef typename timer_service::service_type		service_type;
				typedef typename timer_service::timer_ptr			timer_ptr;

			private:
				timer_service &service_;		// service
				timer_ptr timer_;				// Timerָ��

			public:
				explicit basic_timer_t(service_type &io)
					: service_(timer_service::instance(io))
	
				{}
				// ���ܻص���������ע��һ��Timer
				template<typename HandlerT>
				basic_timer_t(service_type &io, long period, long due, const HandlerT &handler)
					: service_(timer_service::instance(io))
					, timer_(service_.add_timer(period, due, handler))
				{}
				~basic_timer_t()
				{
					if( timer_ )
						service_.erase_timer(timer_);
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
					if( timer_ )
						timer_->set_timer(period, delay);
				}

				// ȡ��Timer
				void cancel()
				{
					if( timer_ )
					{
						service_.erase_timer(timer_);
						//timer_->cancel();
					}
				}

				// ͬ���ȴ�
				void sync_wait()
				{
					assert(timer_);
					timer_->sync_wait();
				}
				template < typename HandlerT >
				void sync_wait(const HandlerT &handler, long period, long delay = 0)
				{
					if( !timer_ )
						timer_ = service_.add_timer(period, delay, handler);

					timer_->sync_wait();
				}

				// �첽�ȴ�
				void async_wait()
				{
					assert(timer_);
					timer_->async_wait();
				}

				template < typename HandlerT >
				void async_wait(const HandlerT &handler, long period, long delay = 0)
				{
					if( !timer_ )
						timer_ = service_.add_timer(period, delay, handler);

					timer_->async_wait();
				}
				
				void expirese_at()
				{
					
				}
			};
		}
	}
}


#endif