#ifndef __IOCP_DISPATCHER_HPP
#define __IOCP_DISPATCHER_HPP

#include <cstdint>
#include <functional>

#include "async_result.hpp"


namespace async { namespace service {
		

		// ��ȡ�ʺ�ϵͳ���߳���
		std::uint32_t get_fit_thread_num(size_t perCPU = 1);



		//------------------------------------------------------------------
		// class io_dispatcher

		class io_dispatcher_t
		{
		public:
			typedef std::function<void()>			init_handler_t;
			typedef std::function<void()>			uninit_handler_t;

		private:
			struct impl;
			std::unique_ptr<impl> impl_;

		public:
			explicit io_dispatcher_t(size_t numThreads = get_fit_thread_num(), const init_handler_t &init = nullptr, const uninit_handler_t &unint = nullptr);
			~io_dispatcher_t();

		private:
			io_dispatcher_t(const io_dispatcher_t &);
			io_dispatcher_t &operator=(const io_dispatcher_t &);

		public:
			// ���豸����ɶ˿�
			void bind(void *);
			// ����ɶ˿�Ͷ������
			template<typename HandlerT>
			void post(HandlerT &&handler);
			// ֹͣ����
			void stop();

		private:
			bool _post_impl(const async_callback_base_ptr &);
		};


		template < typename HandlerT >
		void io_dispatcher_t::post(HandlerT &&handler)
		{
			async_callback_base_ptr async(make_async_callback(std::forward<HandlerT>(handler)));

			if( _post_impl(async) )
				async.release();
		}

	}
}



#endif