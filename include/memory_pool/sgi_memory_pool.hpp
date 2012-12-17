#ifndef __MEMORYPOOL_SGI_HPP
#define __MEMORYPOOL_SGI_HPP


#include "../multi_thread/lock.hpp"
#include "sgi_malloc_pool.hpp"
#include <vector>
#include <algorithm>
#include <objbase.h>



/*
ʵ�ַ���:
���ڴ�ز���HASH-LIST���ݽṹ��������,����һ���ڴ�ʱ,�����Ҫ����ڴ泬����ĳ��������ֱ�ӵ���malloc�����ڴ�, 
�������Ƚ������ݶ���,�����������Ľ���õ����ڵ�HASH��,�ڸ�HASH-LIST�в���ʱ����ڿ��õĽڵ�,
����о�ֱ�ӷ���,����ÿ����20���ڵ�Ԫ��Ϊ������ʼ����LIST�е�Ԫ������,
�����Ȼ����ʧ���˾�ȥ��һ��HASH���в��ҿ����ڴ�,��������
*/



namespace memory_pool
{

	// ���̲߳���Ҫvolatile�����߳�����Ҫ
	template<typename T, bool __IS_MT>
	struct volatile_traits_t
	{
		typedef T* volatile 		value_type;
	};
	template<typename T>
	struct volatile_traits_t<T, false>
	{
		typedef T*					value_type;
	};


	// ��ѡ����
	template<bool __IsMt>
	struct lock_traits_t
	{
		typedef multi_thread::critical_section	value_type;
	};
	template<>
	struct lock_traits_t<false>
	{
		typedef multi_thread::lock_null			value_type;
	};


	// Win32 �Ϸ����ڴ淽ʽ

	struct virtual_traits_t
	{
		void *allocate(size_t size)
		{
			// ��ָ�����ڴ�ҳ��ʼ�ձ����������ڴ��ϣ�����������������ҳ�ļ���
			void *p = ::VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE);
			::VirtualLock(p, size);

			return p;
		}

		void deallocate(void *p, size_t size)
		{
			::VirtualUnlock(p, size);
			::VirtualFree(p, size, MEM_RELEASE);
		}
	};

	struct heap_traits_t
	{
		HANDLE heap_;

		heap_traits_t()
		{
			heap_ = ::HeapCreate(0, 0, 0);
			assert(heap_ != 0);

			// ���õ���Ƭ��
			ULONG uHeapFragValue = 2;
			BOOL suc = ::HeapSetInformation(heap_, HeapCompatibilityInformation, &uHeapFragValue, sizeof(ULONG));
			assert(suc);
		}

		~heap_traits_t()
		{
			BOOL suc = ::HeapDestroy(heap_);
			assert(suc);
		}


		void *allocate(size_t size)
		{
			return ::HeapAlloc(heap_, HEAP_ZERO_MEMORY, size);;
		}

		void deallocate(void *p, size_t/* size*/)
		{
			::HeapFree(heap_, 0, p);
		}
	};

	struct malloc_traits_t
	{
		void *allocate(size_t size)
		{
			return malloc(size);
		}

		void deallocate(void *p, size_t)
		{
			return free(p);
		}
	};

	struct com_traits_t
	{
		void *allocate(size_t size)
		{
			return ::CoTaskMemAlloc(size);
		}

		void deallocate(void *p, size_t)
		{
			return ::CoTaskMemFree(p);
		}
	};


	// С����������� __MAX_BYTES = 256

	template< bool __IS_MT, size_t __MAX_BYTES, typename AllocT = malloc_traits_t >
	class sgi_memory_pool_t
		: public AllocT
	{
	public:
		typedef typename lock_traits_t<__IS_MT>::value_type LockType;
		typedef multi_thread::auto_lock_t<LockType>			AutoLock;

		// ���̹߳���ʱ��Ӧ���ñ�������volatile���Σ������߳�Ӧ�����価���Ż�����ٶ�
		union obj;
		typedef typename volatile_traits_t<obj, __IS_MT>::value_type	ObjPtrType;



	private:
		// С��������ϵ��߽�
		static const size_t __ALIGN = 8;

		// free-lists�ĸ���
		static const size_t __NUM_FREE_LISTS = __MAX_BYTES / __ALIGN;

		// ÿ�γ�ʼ��ʱ������free - list������Ԫ�ص�����
		static const size_t __NUM_NODE = 20;

		// Chunk allocation state
	private:
		// �ڴ����ʼλ��
		char *start_free_;
		// �ڴ�ؽ���λ��
		char *end_free_;
		// ������Ŀռ��С
		size_t heap_size_;

		typedef std::vector<std::pair<void *, size_t>> Bufs;
		Bufs buffers_;

		// �߳���
		LockType mutex_;	

	private:
		// free - lists�Ľڵ㹹��
		union obj
		{
			union obj *pFreeListLink;
			char clientData[1];		/* The client sees this*/
		};

		// free - lists����
		ObjPtrType free_lists_[__NUM_FREE_LISTS];


	public:
		sgi_memory_pool_t()
			: start_free_(0)
			, end_free_(0)
			, heap_size_(0)
		{
			//STATIC_ASSERT(__NUM_FREE_LISTS != 0, __NUM_FREE_LISTS);

			buffers_.reserve(__NUM_FREE_LISTS);
			::memset((void *)free_lists_, 0, __NUM_FREE_LISTS * sizeof(ObjPtrType));
		}

		~sgi_memory_pool_t()
		{
			clear();
		}

	private:
		sgi_memory_pool_t(const sgi_memory_pool_t &);
		sgi_memory_pool_t &operator=(const sgi_memory_pool_t &);

	public:
		// n�������0
		void *allocate(size_t n)
		{
			// ����__MAX_BYTES����MallocAllocator
			if( n > __MAX_BYTES )
				return malloc_pool::allocate(n);

			// Ѱ��free - lists���ʵ���һ��
			ObjPtrType *pFreeListTemp = free_lists_ + FREELISTINDEX(n);
			obj *pResult = NULL;

			// ���ù��캯��ʱ��Ҫ����
			{
				AutoLock lock(mutex_);	

				pResult = *pFreeListTemp;

				if( pResult == NULL )
				{	
					// ���û���ҵ����õ�free - list��׼���������free - list
					pResult = re_fill(ROUNDUP(n));
				}
				else
				{
					// ����free list,ʹ��ָ����һ��List�Ľڵ㣬���������ʱ��ͷ���ΪNULL
					*pFreeListTemp = pResult->pFreeListLink;
				}

			}

			return pResult;
		}


		// p����Ϊ��
		void deallocate(void *p, size_t n)
		{
			// ����__MAX_BYTES����MallocAllocator
			if( n > __MAX_BYTES )
				return malloc_pool::deallocate(p, n);

			// Ѱ��free - lists���ʵ���һ��
			obj *pTemp = reinterpret_cast<obj *>(p);
			ObjPtrType *pFreeListTemp = free_lists_ + FREELISTINDEX(n);

			{
				AutoLock lock(mutex_);

				// ������Ӧ��free - list,���ա��ı�Nextָ�룬�����صĽڵ����List��ͷ
				pTemp->pFreeListLink = *pFreeListTemp;
				*pFreeListTemp = pTemp;
			}

		}


		void *reallocate(void *p, size_t szOld, size_t szNew)
		{
			// �������__MAX_BYTES����MallocAllocator
			if( szOld > __MAX_BYTES && szNew > __MAX_BYTES) 
			{
				return malloc_pool::reallocate(p, szOld, szNew);
			}

			// ���Բ����Ĵ�С��ͬ��ֱ�ӷ���
			if( ROUNDUP(szOld) == ROUNDUP(szNew) ) 
				return p;

			// �ٴ�����ռ�
			void *pResult = allocate(szNew);

			// �ж�ѡ����������
			size_t szcopy = szNew > szOld ? szOld : szNew;
			::memmove(pResult, p, szcopy);

			// ɾ��ԭ�ȵ�����
			deallocate(p, szOld);

			return pResult;

		}


	private:
		// ROUND_UP ��bytes�ϵ���__ALIGN�ı���
		static inline size_t ROUNDUP(size_t bytes)
		{
			return ((bytes) + __ALIGN - 1) & ~(__ALIGN - 1);
		}

		// ���������С������ʹ�õ�n��free - lists��n��0����
		static inline size_t FREELISTINDEX(size_t bytes)
		{
			return (((bytes) + __ALIGN - 1) / __ALIGN) - 1;
		}

	private:
		// ����һ����СΪn�Ķ���,�������СΪn���������鵽free - list
		obj *re_fill(size_t n)
		{
			// ȱʡΪ__NUM_NODE��������,����ڴ�ռ䲻�㣬��õ����������С��20
			size_t nObjs = __NUM_NODE;

			// ����ChunkAlloc,����ȡ��nObjs������Ϊfree - list��������
			// nObjs����Pass By reference����
			char *pChunk = chunk_alloc(n, nObjs);

			// ���ֻ���һ������,���������ͷ���������ߣ�free - list��������
			if( 1 == nObjs )
				return reinterpret_cast<obj *>(pChunk);

			// �������free - list��ע��������
			ObjPtrType *pFreeListTemp  = free_lists_ + FREELISTINDEX(n);

			// ��Chunk�ռ��ڽ���free - list
			// pResult׼�����ظ��ͻ���
			obj *pResult = reinterpret_cast<obj *>(pChunk);

			obj *pCurObj = NULL, *pNextObj = NULL;

			// ������һ����λ���ڴ棬����һ������
			--nObjs;
			// ��Ҫ����һ����λ���ڴ棬����һ����λ��ʼ��ʣ���obj��������, ����free - listָ�������ÿռ�
			*pFreeListTemp = pNextObj = reinterpret_cast<obj *>(pChunk + n);

			// ��free - list�ĸ����鴮������
			// ��1��ʼ,��0������
			for(size_t i = 1; ; ++i)
			{
				pCurObj = pNextObj;
				pNextObj = reinterpret_cast<obj *>(reinterpret_cast<char *>(pNextObj) + n);

				if( nObjs == i )
				{
					// �������, ��һ���ڵ�ΪNULL, �˳�ѭ��
					pCurObj->pFreeListLink = NULL;
					break;
				}
				else
				{
					pCurObj->pFreeListLink = pNextObj;
				}
			}

			return pResult;
		}

		// ����һ���ռ�,������nObjs����СΪsize������
		// ���䵥λ�ߴ�Ϊsize, ��nObjs��Ԫ��
		// ��Щ�ڴ��������ַ��������һ���, ������ָ��
		char *chunk_alloc(size_t sz, size_t &nObjs)
		{
			size_t szTotal = sz * nObjs;
			// �ڴ��ʣ��ռ�
			size_t szLeft =  end_free_ - start_free_;

			char *pResult = NULL;
			if( szLeft >= szTotal )
			{
				// �ڴ��ʣ��ռ���������
				pResult = start_free_;

				// �ƶ�ָ��ʣ��ռ��ָ��
				start_free_ += szTotal;

				return pResult;
			}
			else if( szLeft >= sz )
			{
				// �ڴ��ʣ��ռ䲻����ȫ���������������㹻һ�����ϵ�����
				// �ı�����Ĵ�С
				nObjs = szLeft / sz;

				// �ƶ�ָ��ʣ��ռ��ָ��
				szTotal = sz * nObjs;
				pResult = start_free_;
				start_free_ += szTotal;

				return pResult;
			}
			else 
			{
				// �������ڴ���е�ʣ�໹�����ü�ֵ
				if( szLeft > 0 )
				{
					// ������ʵ���free - list, Ѱ���ʵ���free - list
					ObjPtrType *pFreeListTemp = free_lists_ + FREELISTINDEX(szLeft);

					// ����free - list�����ڴ���еĲ���ռ����
					reinterpret_cast<obj *>(start_free_)->pFreeListLink = *pFreeListTemp;
					*pFreeListTemp = reinterpret_cast<obj *>(start_free_);
				}

				// �ڴ��ʣ��ռ䲻��һ������
				// ��Ҫ��ȡ���ڴ�, ע���һ�η��䶼Ҫ������szTotal�Ĵ�С
				// ͬʱҪ����ԭ�е�m_szHeap / 4�Ķ���ֵ
				size_t szGet = 2 * szTotal + ROUNDUP(heap_size_ >> 4);

				// ����Heap�ռ䣬���������ڴ��
				start_free_ = reinterpret_cast<char *>(AllocT::allocate(szGet));

				if( NULL == start_free_ )
				{
					//// Heap�ռ䲻��
					//ObjPtrType *pFreeList = NULL; 
					//obj *pTemp = NULL;

					//for(int i = sz; i <= __MAX_BYTES; i += __ALIGN)
					//{
					//	pFreeList = m_pFreeLists + FREELISTINDEX(i);
					//	pTemp = *pFreeList;

					//	if( NULL == pTemp )
					//	{
					//		// free - list����δ�����飬���������ͷ�
					//		*pFreeList = pTemp->pFreeListLink;
					//		m_pStartFree = reinterpret_cast<char *>(pTemp);
					//		m_pEndFree = m_pStartFree + i;

					//		// �ݹ���ã�����nObjs
					//		return ChunkAlloc(sz, nObjs);
					//	}
					//}

					// û�з��䵽�ڴ棬ת��MallocMemoryPool
					end_free_ = 0;
					start_free_ = reinterpret_cast<char *>(malloc_pool::allocate(szGet));
				}

				// �洢�����ṩ�ͷ�
				buffers_.push_back(std::make_pair(start_free_, szGet));

				heap_size_ += szGet;
				end_free_ = start_free_ + szGet;

				// �ݹ���ã�����nObjs
				return chunk_alloc(sz, nObjs);
			}
		}

		// ����ڴ�
		void clear()
		{
			for(Bufs::iterator iter = buffers_.begin();
				iter != buffers_.end(); ++iter)
			{
				AllocT::deallocate(iter->first, iter->second);
			}
			buffers_.clear();
		}
	};


	typedef sgi_memory_pool_t<true, 256>					mt_malloc_memory_pool;
	typedef sgi_memory_pool_t<false, 256>					st_malloc_memory_pool;

	typedef sgi_memory_pool_t<true, 256, virtual_traits_t>	mt_virtual_memory_pool;
	typedef sgi_memory_pool_t<false, 256, virtual_traits_t>	st_virtual__memory_pool;

	typedef sgi_memory_pool_t<true, 256, heap_traits_t>		mt_heap_memory_pool;
	typedef sgi_memory_pool_t<false, 256, heap_traits_t>	st_heap_memory_pool;

	typedef sgi_memory_pool_t<true, 256, com_traits_t>		mt_com_memory_pool;
	typedef sgi_memory_pool_t<false, 256, com_traits_t>		st_com_memory_pool;

	
	typedef mt_malloc_memory_pool							mt_memory_pool;
	typedef st_malloc_memory_pool							st_memory_pool;
}


#endif