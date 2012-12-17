#ifndef ___MEMORYPOOL_SGI_MALLOCHPP
#define ___MEMORYPOOL_SGI_MALLOCHPP


#include <new>
#include <memory>
#include <exception>



namespace memory_pool
{


	class malloc_pool
	{
		// ����set_new_handler(),ָ���Լ���out-of-memory handler
		typedef void (*pFuncOOMHandler)();

	private:
		static pFuncOOMHandler m_pFuncOOMHandler;

	private:
		// ���º������������ڴ治��
		// oom : out of memory
		static void *out_of_malloc(size_t sz)
		{
			pFuncOOMHandler pHandler = NULL;
			void *pResult = NULL;

			// ���ϳ����ͷš����á����ͷš�������...
			for( ; ; )
			{
				pHandler = m_pFuncOOMHandler;

				if( NULL == pHandler )
					throw std::bad_alloc();

				// ���ô�����,��ͼ�ͷ��ڴ�
				(*pHandler)();

				// �ٴγ��������ڴ�
				pResult = malloc(sz);
				if( pResult )
					return pResult;
			}
		}


		static void *out_of_realloc(void *p, size_t sz)
		{
			pFuncOOMHandler pHandler = NULL;
			void *pResult = NULL;

			// ���ϳ����ͷš����á����ͷš�������...
			for( ; ; )
			{
				pHandler = m_pFuncOOMHandler;

				if( NULL == pHandler )
					throw std::bad_alloc();

				// ���ô�����,��ͼ�ͷ��ڴ�
				(*pHandler)();

				// �ٴγ��������ڴ�
				pResult = std::realloc(p, sz);
				if( pResult )
					return pResult;
			}
		}



	public:
		static void *allocate(size_t sz)
		{
			// ֱ��ʹ��malloc
			void *pResult = malloc(sz);

			// ����޷�����Ҫ��ʱ,����OOMMalloc
			if( NULL == pResult )
				pResult = out_of_malloc(sz);

			return pResult;
		}

		static void deallocate(void *p, size_t/* sz*/)
		{
			// ֱ��ʹ��free
			free(p);
		}


		static void *reallocate(void *p, size_t/* szOld*/, size_t szNew)
		{
			// ֱ��ʹ��realloc
			void *pResult = std::realloc(p, szNew);

			// �޷�����Ҫ��ʱ, ����OOMRealloc
			if( NULL == pResult )
				pResult = out_of_realloc(p, szNew);

			return pResult;
		}


		// ����set_new_handler(),ָ���Լ���out-of-memory handler
		static void (*malloc_pool::pFuncSetOOMHandler(pFuncOOMHandler pFunc))()
		{
			pFuncOOMHandler pOldFunc = m_pFuncOOMHandler;
			m_pFuncOOMHandler = pFunc;

			return pOldFunc;
		}
	};

	// __declspec(selectany) ��ֹLNK2005
	__declspec(selectany) malloc_pool::pFuncOOMHandler malloc_pool::m_pFuncOOMHandler = NULL;
}



#endif