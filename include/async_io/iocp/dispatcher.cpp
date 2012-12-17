#include "dispatcher.hpp"
#include "process.h"

#include <iostream>

namespace async
{
	
	namespace iocp
	{

		io_dispatcher::io_dispatcher(size_t numThreads/* = 0*/, const InitCallback &init/* = 0*/, const UninitCallback &unint/* = 0*/)
			: init_handler_(init)
			, uninit_handler_(unint)
		{
			if( !iocp_.create(numThreads) )
				throw win32_exception("iocp_.Create()");

			// ����ָ�����߳���
			threads_.reserve(numThreads);

			try
			{
				for(int i = 0; i != numThreads; ++i)
				{
					HANDLE hThread = ::CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(_thread_io_impl), this, 0, 0);//(HANDLE)::_beginthreadex(NULL, 0, &io_dispatcher::_thread_io_impl, this, 0, 0);

					if( hThread == NULL )
						throw win32_exception("CreateThread");

					threads_.push_back(hThread);
				}
			}
			catch(...)
			{
				stop();
				throw;
			}
		}

		io_dispatcher::~io_dispatcher()
		{
			try
			{		
				stop();
				iocp_.close();
			}
			catch(...)
			{
				assert(0 && __FUNCTION__);
				std::cerr << "Unknown error!" << std::endl;
			}
		}


		void io_dispatcher::bind(HANDLE hHandle)
		{
			if( !iocp_.associate_device(hHandle, 0) )
				throw win32_exception("iocp_.associate_device");
		}


		void io_dispatcher::stop()
		{
			// ��ֹͣ���е��߳�
			std::for_each(threads_.begin(), threads_.end(), 
				[this](HANDLE handle){ iocp_.post_status(0, 0, NULL); });

			// �ȴ��߳��˳����رվ��
			if( !threads_.empty() )
				::WaitForMultipleObjects(threads_.size(), &threads_[0], TRUE, INFINITE);

			std::for_each(threads_.begin(), threads_.end(), std::ptr_fun(::CloseHandle));

			threads_.clear();
		}

		void io_dispatcher::_thread_io()
		{
			multi_thread::call_stack_t<io_dispatcher>::context ctx(this);

			DWORD dwSize			= 0;
			ULONG_PTR uKey			= 0;
			OVERLAPPED *pOverlapped = 0;

			ULONG_PTR *key			= &uKey;

			while(true)
			{
				::SetLastError(0);
				bool bSuc = iocp_.get_status(reinterpret_cast<ULONG_PTR *>(&key), &dwSize, &pOverlapped);
				u_long err = ::GetLastError();

				// �����˳�
				if( key == 0 && pOverlapped == 0 )
					break;

				try
				{
					// �ص�
					async_callback_base::call(key, pOverlapped, dwSize, err);
				}
				catch(const exception::exception_base &e)
				{
					e.dump();
					std::cerr << e.what() << std::endl;
					assert(0);
				}
				catch(const std::exception &e)
				{
					std::cerr << e.what() << std::endl;
					assert(0);
					// Opps!!
				}
				catch(...)
				{
					assert(0);
					// Opps!!
				}
				
			}
		}


		size_t io_dispatcher::_thread_io_impl(LPVOID pParam)
		{
			io_dispatcher *pThis = reinterpret_cast<io_dispatcher *>(pParam);

			if( pThis->init_handler_ != 0 )
				pThis->init_handler_();

			pThis->_thread_io();

			if( pThis->uninit_handler_ != 0 )
				pThis->uninit_handler_();

			::OutputDebugStringW(L"OVERLAPPED Thread Exit\n");
			return 0;
		}


	}

}