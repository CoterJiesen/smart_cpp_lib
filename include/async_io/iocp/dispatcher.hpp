#ifndef __IOCP_DISPATCHER_HPP
#define __IOCP_DISPATCHER_HPP


#include "iocp.hpp"
#include "async_result.hpp"
#include "win_exception.hpp"
#include "../../multi_thread/tls.hpp"

#include <vector>
#include <type_traits>


namespace async
{

	namespace iocp 
	{
		

		// ��ȡ�ʺ�ϵͳ���߳���
		inline size_t get_fit_thread_num(size_t perCPU = 2)
		{
			SYSTEM_INFO systemInfo = {0};
			::GetSystemInfo(&systemInfo);

			return perCPU * systemInfo.dwNumberOfProcessors + 2;
		}



		//------------------------------------------------------------------
		// class io_dispatcher
		// ��ɶ˿�ʵ��

		class io_dispatcher
		{
		public:
			// �߳���������
			typedef std::vector<HANDLE>				threads_type;
			typedef threads_type::const_iterator	threads_const_iterator;
			typedef std::function<void()>			InitCallback;
			typedef std::function<void()>			UninitCallback;

		private:
			// iocp Handle
			iocp_handle iocp_;
			// �߳�����
			std::vector<HANDLE>	threads_;
			// �̴߳������ʼ������
			InitCallback init_handler_;
			// �߳��˳�ʱ��������
			UninitCallback uninit_handler_;

		public:
			explicit io_dispatcher(size_t numThreads = get_fit_thread_num(), const InitCallback &init = 0, const UninitCallback &unint = 0);
			~io_dispatcher();

		public:
			// ���豸����ɶ˿�
			void bind(HANDLE);
			// ����ɶ˿�Ͷ������
			template<typename HandlerT>
			void post(const HandlerT &handler);
			// ���������̳߳���ʱ����������з���
			template<typename HandlerT>
			void dispatch(const HandlerT &handler);

			// ֹͣ����
			void stop();

		private:
			void _thread_io();

		private:
			static size_t WINAPI _thread_io_impl(LPVOID);
		};


		template < typename HandlerT >
		void io_dispatcher::post(const HandlerT &handler)
		{
			async_callback_base_ptr async(make_async_callback<async_callback>(handler));

			if( !iocp_.post_status(0, 0, static_cast<OVERLAPPED *>(async.get())) )
				throw win32_exception("iocp_.PostStatus");

			async.release();
		}

		template < typename HandlerT >
		void io_dispatcher::dispatch(const HandlerT &handler)
		{
			if( multi_thread::call_stack_t<io_dispatcher>::contains(this) )
			{
				async_callback_base_ptr async(make_async_callback<async_callback>(handler));
				
				ULONG key = 0;
				ULONG_PTR *key_tmp = &key;
				async_callback_base::call(&key_tmp, async.get(), 0, 0);
			
				async.release();
			}
			else
				post(handler);
		}
	}
}



#endif