#ifndef __MULTI_THREAD_THREADPOOL_HPP
#define __MULTI_THREAD_THREADPOOL_HPP

#pragma warning(disable:4100)	// ����δ�����βξ���

/*
����
	set_threadpool_max_num

�����ڴ�hook����
	handler_allocate_hook
	handler_deallocate_hook

ϵͳ�̳߳�
	queue_work_item_pool
	queue_timer_pool
	io_completion_pool
	wait_object_pool

todo:
	VISTA����ϵͳ�̳߳�
*/


namespace multi_thread
{

	/*
	�̳߳ص����������Ϊ���ԣ�

	Timer						RegisterWait					BindIO							QueueUser

	�̵߳ĳ�ʼ��ֵ		����1						0								0								0

	������һ���߳�ʱ		�����õ�һ���̳߳غ���ʱ		ÿ63��ע�����һ���߳�				ϵͳ��ʹ����̽��������������Ӱ�죺
	1. �Դ�����̺߳��Ѿ���ȥһ��ʱ��(��λms)
	2. ʹ��WT_EXECUTELONGFUNCTION
	3. �Ѿ��ŶӵĹ�����Ŀ������������ĳ����ֵ

	���̱߳�����ʱ		��������ֹʱ					���Ѿ�ע��ĵȴ���������Ϊ0		���߳�û��δ�����IO����
	�����Ѿ�������һ����ֵ������		���߳̿�����һ����ֵ������

	�߳���εȴ�			�����ȴ�						WaitForMultipleObjects			�����ȴ�							GetQueuedCompletionStatus

	ʲô�����߳�			�ȴ���ʱ��֪ͨ�Ŷӵ�APC		�ں˶����Ϊ��֪ͨ״̬				�Ŷӵ��û�APC������ɵ�IO����		չʾ����ɵ�״̬��IO��ʾ




	*/




	/*
	WT_EXECUTEDEFAULT-- Ĭ������£��ûص��������Ŷӵ��� i/o �����̡߳���ͨ��IO��ɶ˿�ʵ�ֵ�
	�ص��������Ŷӣ�����ζ�����ǲ���ִ��һ�� alertable �ĵȴ� I/O ��ɶ˿�ʹ�õ��̡߳� 
	��ˣ������� I/O ������ APC���������ڵȴ���Ϊ���ܱ�֤���߳��ڻص���ɺ󽫽��� alertable �ȴ�״̬��

	WT_EXECUTEINIOTHREAD--�ص�������һ�� I/O �����߳����Ŷӡ� ���߳̽�ִ��һ�� alertable �ĵȴ��� 
	����Ч�ʽϵͣ���ˣ������ص�����ǰ�߳����� apc �ͺ���̷߳��ص��̳߳أ���Ӧִ�� APC ʱ����Ӧʹ�ô˱�־�� 
	�ص���������Ϊ APC �Ŷӡ� ����ú���ִ�� alertable �ȴ�����������ؽ�������������⡣

	WT_EXECUTEINPERSISTENTTHREAD--�ص��������Ŷӵ���Զ������ֹ���̡߳� �����ܱ�֤��ͬһ���߳���ÿ��ʹ�á�
	�˱�־Ӧ�����ڶ����񣬻������ܻ�Ӱ��������ʱ�������� 
	��ע�⵱ǰû�й��������߳��������־ã���������κι��� I/O ���󣬲�����ֹ�����̡߳�

	WT_EXECUTELONGFUNCTION--�ص���������ִ��һ����ʱ��ĵȴ��� �˱�־���԰������������Ӧ����һ�����̵߳�ϵͳ��

	WT_TRANSFER_IMPERSONATION--�ص�������ʹ���ڵ�ǰ�ķ������ƣ�����һ�����̻���ģ�����ơ�
	���δָ���˱�־����ص�������ִ�н������ơ�

	*/

	enum { MAX_THREADS = 10 };

	// ��������̳߳�����
	inline void set_threadpool_max_num(DWORD dwFlag, DWORD dwThreads = MAX_THREADS)
	{
		WT_SET_MAX_THREADPOOL_THREADS(dwFlag, dwThreads);
	}

	// Handler Allocate / Deallocate HOOK

	inline void *handler_allocate_hook(size_t sz, ...)
	{
		return ::operator new(sz);
	}
	inline void handler_deallocate_hook(size_t sz, void *p, ...)
	{
		sz;
		::operator delete(p);
	}




	namespace detail
	{
		struct param_base
		{
			param_base()
			{}

			virtual ~param_base()
			{}

			void invoke()
			{
				callback(FALSE);
			}
			void invoke(BOOLEAN wait)
			{
				callback(wait);
			}

			virtual void callback(BOOLEAN wait) = 0;
		};

		typedef std::tr1::shared_ptr<param_base> param_base_ptr;

		template < typename HandlerT >
		struct thread_param_t
			: public param_base
		{
			HandlerT handler_;

			thread_param_t(const HandlerT &handler)
				: param_base()
				, handler_(handler)
			{}

			virtual void callback(BOOLEAN wait)
			{
				handler_(wait);
			}
		};


		// handler ������
		template<typename T>
		void _HandlerDeallocate(T *p)
		{
			p->~T();
			handler_deallocate_hook(sizeof(T), p, &p);
		}

		template<typename HandlerT>
		detail::param_base_ptr _HandlerAllocate(const HandlerT &handler)
		{
			typedef detail::thread_param_t<HandlerT> Thread;
			void *p = 0;
			p = handler_allocate_hook(sizeof(Thread), p);
			new (p) Thread(handler);

			detail::param_base_ptr threadParam(static_cast<detail::param_base *>(p), _HandlerDeallocate<detail::param_base>);
			return threadParam;
		}

	}



	//----------------------------------------------------------------------------------
	// class QueueWorkItem

	class queue_work_item_pool
	{
	private:
		detail::param_base_ptr callback_;

	public:
		queue_work_item_pool()
		{}
		template<typename HandlerT>
		explicit queue_work_item_pool(const HandlerT &handle)
			: callback_(detail::_HandlerAllocate(handle))
		{}

	public:
		template<typename HandlerT>
		bool call(const HandlerT &handler, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			if( callback_ == 0 )
				callback_ = detail::_HandlerAllocate(handler);

			assert(callback_);
			return call(nFlags);
		}

		bool call(ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			assert(callback_);
			return TRUE == ::QueueUserWorkItem(&queue_work_item_pool::proc_impl, callback_.get(), nFlags);
		}

	private:
		static DWORD WINAPI proc_impl(LPVOID pvParam)
		{
			detail::param_base *p(static_cast<detail::param_base *>(pvParam));

			try 
			{
				p->invoke();
			}
			catch(...) 
			{}

			return 0;
		}

	private:
		queue_work_item_pool(const queue_work_item_pool &);
		queue_work_item_pool &operator=(const queue_work_item_pool &);
	};



	//----------------------------------------------------------------------------------
	// class QueueTimer

	/*
	WT_EXECUTEDEFAULT				�÷�IO������߳�����������Ŀ
	WT_EXECUTEINIOTHREAD			�����Ҫ��ĳ��ʱ�䷢��һ���첽IO����
	WT_EXECUTEPERSISTENTTHREAD		�����Ҫ��һ����������ֹ���е��߳�������ù�����Ŀ
	WT_EXECUTELONGFUNCTION			�����Ϊ������Ŀ��Ҫ�ܳ�ʱ��������


	WT_EXECUTEDEFAULT
	0x00000000 Ĭ������£��ص��������Ŷӵ��� i/o �����̡߳�

	WT_EXECUTEINTIMERTHREAD
	0x00000020 �ɱ���ļ�ʱ���̵߳��ûص������� �˱�־Ӧ�����ڶ����񣬻������ܻ�Ӱ��������ʱ�������� �ص���������Ϊ APC �Ŷӡ� ����Ӧִ�б����Ȳ�����

	WT_EXECUTEINIOTHREAD
	0x00000001 �ص��������Ŷ���һ�� I/O �����̡߳� ����ú���Ӧִ���̵߳ȴ�����״̬��ʱ����Ӧʹ�ô˱�־�� �ص���������Ϊ APC �Ŷӡ� ����ú���ִ�б����Ȳ�����������ؽ�������������⡣

	WT_EXECUTEINPERSISTENTTHREAD
	0x00000080 �ص��������Ŷӵ���Զ������ֹ���̡߳� �������ܱ�֤��ͬһ�߳���ÿ��ʹ�á� �˱�־Ӧ�����ڶ����񣬻������ܻ�Ӱ��������ʱ�������� ��ע��Ŀǰû�й��������߳��������־���Ȼû�й��������߳̽���ֹ������κι���� I/O ����

	WT_EXECUTELONGFUNCTION
	0x00000010 �ص���������ִ��һ����ʱ��ĵȴ��� �˱�־�ɰ���ϵͳ�������Ƿ���Ӧ����һ���µ��̡߳�

	WT_EXECUTEONLYONCE
	0x00000008 ��ʱ����һ��ֻ������Ϊ�ѷ����ź�״̬�� ��������˴˱�־ �� ��������Ϊ�㡣

	WT_TRANSFER_IMPERSONATION
	0x00000100 �ص�������ʹ�õ�ǰ�ķ������ƣ������ǽ��̻�ģ�����ơ� �����ָ���˱�־����ص�����ִֻ�н������ơ�

	*/
	class queue_timer_pool
	{
		detail::param_base_ptr callback_;
		HANDLE newTimer_;

	public:
		queue_timer_pool()
			: newTimer_(0)
		{}
		template<typename HandlerT>
		explicit queue_timer_pool(const HandlerT &handler)
			: callback_(detail::_HandlerAllocate(handler))
			, newTimer_(NULL)
		{}

		~queue_timer_pool()
		{
			if( newTimer_ != NULL )
				cancel();

			assert(callback_);
		}

	public:
		template < typename HandlerT >
		bool call(const HandlerT &handler, DWORD dwDueTime, DWORD dwPeriod, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			if( *callback_ == 0 )
				callback_ = detail::_HandlerAllocate(handler);

			assert(handler);
			return call(dwDueTime, dwPeriod, nFlags);
		}

		bool call(DWORD dwDueTime, DWORD dwPeriod, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			assert(callback_);
			return TRUE == ::CreateTimerQueueTimer(&newTimer_, NULL, 
				reinterpret_cast<WAITORTIMERCALLBACK>(&queue_timer_pool::proc_impl), 
				callback_.get(), dwDueTime, dwPeriod, nFlags);
		}

		// �ı䶨ʱ���ĵ���ʱ��͵�������
		bool change(DWORD dwDueTime, DWORD dwPeriod)
		{
			assert(newTimer_ != NULL);
			assert(callback_);
			return TRUE == ::ChangeTimerQueueTimer(NULL, newTimer_, dwDueTime, dwPeriod);
		}

		// ��Ӧ��ΪhCompletionEvent����INVALID_HANDLE_VALUE
		// ����Ǵ��ݵ�NULL����ú������Ϸ���
		// ����Ǵ��ݵ�һ���¼������������Ϸ��أ����ҵ���ʱ�������Ѿ��ŶӵĹ�����Ŀ��ɺ󣬻ᴥ�����¼�(��Ӧ����Ϊ�������¼�)
		bool cancel(HANDLE hCompletionEvent = NULL)
		{
			assert(newTimer_ != NULL);
			assert(callback_);
			if( ::DeleteTimerQueueTimer(NULL, newTimer_, hCompletionEvent) )
			{
				newTimer_ = NULL;
				return true;
			}

			return false;
		}

	private:
		static void WINAPI proc_impl(PVOID pvParam, BOOL bTimeout)
		{
			detail::param_base *p = static_cast<detail::param_base *>(pvParam);

			try 
			{
				p->invoke(static_cast<BOOLEAN>(bTimeout));
			}
			catch(...) 
			{}
		}

	private:
		queue_timer_pool(const queue_timer_pool &);
		queue_timer_pool &operator=(const queue_timer_pool &);
	};



	/*
	Ͷ��һ���첽IO����,��IO��ɶ˿��ϣ��ص�����Ҳ�����̳߳��߳���ִ��

	������Ӧ�ó��򷢳�ĳЩ�첽IO���󣬵���Щ�������ʱ����Ҫ��һ���̳߳�׼��������������ɵ�IO����
	BindIoCompletionCallback���ڲ�����CreateIoCompletionPort������hDevice���ڲ���ɶ˿ڵľ����
	���øú������Ա�֤������һ���߳�ʼ���ڷ�IO����У����豸��ص���ɼ����ص�������̵ĵ�ַ��
	�����������豸��IO�������ʱ����IO�����֪�������ĸ��������Ա��ܹ����������IO����


	*/

	//----------------------------------------------------------------------------------
	// class IOCompletionPool 

	class io_completion_pool
	{
		detail::param_base_ptr callback_;

	public:
		template<typename HandlerT>
		io_completion_pool(const HandlerT &handler)
			: callback_(detail::_HandlerAllocate(handler))
		{}

	private:
		io_completion_pool(const io_completion_pool &);
		io_completion_pool &operator=(const io_completion_pool &);

	public:
		bool call(HANDLE hBindHandle)
		{
			return ::BindIoCompletionCallback(hBindHandle, 
				&io_completion_pool::proc_impl, 0) == TRUE;
		}

	private:
		static void WINAPI proc_impl(DWORD dwError, DWORD dwNum, OVERLAPPED *pOverlapped)
		{
			// .... to do
			//static_assert(0, "to do");
		}
	};



	/*
	WT_EXECUTEDEFAULT	0x00000000 
	Ĭ������£��ûص��������Ŷӵ��� i/o �����̡߳�

	WT_EXECUTEINIOTHREAD	0x00000001 
	�ص�������һ�� I/O �����߳����Ŷӡ� ����ú���Ӧ�ȴ� alertable ״̬���߳���ִ�У�Ӧʹ�ô˱�־�� 
	�ص���������Ϊ APC �Ŷӡ� ����ú���ִ�� alertable �ȴ�������������ؽ�������������⡣

	WT_EXECUTEINPERSISTENTTHREAD 0x00000080 
	�ص��������Ŷӵ���Զ������ֹ���̡߳� �����ܱ�֤��ͬһ���߳���ÿ��ʹ�á� �˱�־Ӧ�����ڶ����񣬻������ܻ�Ӱ�������ȴ������� 
	��ע�⵱ǰû�й��������߳��������־õ���Ȼû�й��������̣߳�������κι��� I/O �������ֹ��

	WT_EXECUTEINWAITTHREA 0x00000004 
	�ɱ���ĵȴ��̵߳��ûص������� �˱�־Ӧ�����ڶ����񣬻������ܻ�Ӱ�������ȴ������� 
	�����һ���̻߳�ȡ�������͵��ûص�����ʱ�� UnregisterWait �� UnregisterWaitEx ��������ͼ��ȡ��ͬ�������ͻᷢ��������

	WT_EXECUTELONGFUNCTION	0x00000010 
	�ص���������ִ��һ����ʱ��ĵȴ��� �˱�־���԰������������Ӧ����һ�����̵߳�ϵͳ��

	WT_EXECUTEONLYONCE	0x00000008 
	�ص���������һ�θ��߳̽����ٵȴ��þ���� ����ÿ�εȴ��������֮ǰ�ȴ�������ȡ���������ü�ʱ����

	WT_TRANSFER_IMPERSONATION	0x00000100 
	�ص�������ʹ���ڵ�ǰ�ķ������ƣ�����һ�����̻���ģ�����ơ� ���δָ���˱�־����ص�������ִ�н������ơ�

	*/

	//----------------------------------------------------------------------------------
	// class WaitObjectPool

	class wait_object_pool
	{
	private:
		detail::param_base_ptr callback_;
		HANDLE waitObject_;

	public:
		wait_object_pool()
			: waitObject_(0)
		{}
		template<typename HandlerT>
		explicit wait_object_pool(const HandlerT &handler)
			: callback_(detail::_HandlerAllocate(handler))
			, waitObject_(0)
		{}
		~wait_object_pool()
		{

		}

	private:
		wait_object_pool(const wait_object_pool &);
		wait_object_pool &operator=(const wait_object_pool &);


	public:
		template<typename HandlerT>
		bool call(const HandlerT &handler, HANDLE hObject, ULONG dwWait = INFINITE, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			if( *callback_ == 0 )
				callback_ = detail::_HandlerAllocate(handler);

			assert(*callback_);
			return call(hObject, dwWait, nFlags);
		}

		bool call(HANDLE hObject, ULONG dwWait = INFINITE, ULONG nFlags = WT_EXECUTEDEFAULT)
		{
			assert(callback_);
			return TRUE == ::RegisterWaitForSingleObject(&waitObject_, hObject, 
				&wait_object_pool::proc_impl, callback_.get(), dwWait, nFlags);
		}	
		// ȡ��
		bool cancel(HANDLE hCompletion = NULL)
		{
			assert(callback_);
			assert(waitObject_ != NULL);
			return TRUE == ::UnregisterWaitEx(waitObject_, hCompletion);
		}


	private:
		static void WINAPI proc_impl(PVOID pvParam, BOOLEAN bTimerOrWaitFired)
		{
			detail::param_base *p(static_cast<detail::param_base *>(pvParam));

			try 
			{
				p->invoke(bTimerOrWaitFired);
			}
			catch(...) 
			{}
		}
	};


}

#endif //