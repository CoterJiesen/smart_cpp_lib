#ifndef __MEMORY_FIXED_MEMORY_POOL_HPP
#define __MEMORY_FIXED_MEMORY_POOL_HPP


#include "sgi_memory_pool.hpp"


namespace memory_pool
{


	// ������С__BYTES = 10 * 1024

	template<bool __IS_MT, size_t __BYTES, typename AllocT = malloc_traits_t>
	class fixed_memory_pool_t
		: AllocT
	{
	public:
		typedef typename lock_traits_t<__IS_MT>::value_type LockType;
		typedef std::lock_guard<LockType>					AutoLock;
		typedef AllocT										AllocType;

		// ���̹߳���ʱ��Ӧ���ñ�������volatile���Σ������߳�Ӧ�����価���Ż�����ٶ�
		union obj;
		typedef typename volatile_traits_t<obj, __IS_MT>::value_type	ObjPtrType;


	private:
		// ÿ�γ�ʼ��ʱ������free - list������Ԫ�ص�����
		static const size_t __NUM_NODE = 20;
		static const size_t __ALIGN = 4;

		// ROUND_UP ��bytes�ϵ���__ALIGN�ı���
		enum { ROUND = (__BYTES + __ALIGN - 1) & ~(__ALIGN - 1) };

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
		ObjPtrType free_lists_[1];


	public:
		fixed_memory_pool_t()
			: start_free_(0)
			, end_free_(0)
			, heap_size_(0)
		{
			buffers_.reserve(__NUM_NODE);
			::memset((void *)free_lists_, 0, 1 * sizeof(ObjPtrType));
		}

		~fixed_memory_pool_t()
		{
			clear();
		}

	private:
		fixed_memory_pool_t(const fixed_memory_pool_t &);
		fixed_memory_pool_t &operator=(const fixed_memory_pool_t &);

	public:
		void *allocate(size_t n)
		{
			assert(n <= __BYTES);

			// Ѱ��free - lists���ʵ���һ��
			ObjPtrType *pFreeListTemp = free_lists_;
			obj *pResult = NULL;

			// ���ù��캯��ʱ��Ҫ����
			{
				AutoLock lock(mutex_);	

				pResult = *pFreeListTemp;

				if( pResult == NULL )
				{	
					// ���û���ҵ����õ�������
					pResult = re_fill(ROUND);
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
		void deallocate(void *p, size_t)
		{
			// �õ��ڴ�ص�ַ
			obj *pTemp = reinterpret_cast<obj *>(p);
			ObjPtrType *pFreeListTemp = free_lists_;

			{
				AutoLock lock(mutex_);

				// ���ա��ı�Nextָ�룬�����صĽڵ����List��ͷ
				pTemp->pFreeListLink = *pFreeListTemp;
				*pFreeListTemp = pTemp;
			}

		}

	private:
		// ROUND_UP ��bytes�ϵ���__ALIGN�ı���
		static inline size_t ROUNDUP(size_t bytes)
		{
			return ((bytes) + __ALIGN - 1) & ~(__ALIGN - 1);
		}

	private:
		// ����һ����СΪn�Ķ���,�������СΪn���������鵽free - list
		obj *re_fill(size_t n)
		{
			// ȱʡΪ__NUM_NODE��������,����ڴ�ռ䲻�㣬��õ����������С��20
			size_t nObjs = __NUM_NODE;

			// ����ChunkAlloc,����ȡ��nObjs������
			// nObjs����Pass By reference����
			char *pChunk = chunk_alloc(n, nObjs);

			// ���ֻ���һ������,���������ͷ����������,���¿�������
			if( 1 == nObjs )
				return reinterpret_cast<obj *>(pChunk);

			// �������free - list��ע��������
			ObjPtrType *pFreeListTemp  = free_lists_;

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
				// �ڴ��ʣ��ռ䲻��һ������
				// ��Ҫ��ȡ���ڴ�, ע���һ�η��䶼Ҫ������szTotal�Ĵ�С
				// ͬʱҪ����ԭ�е�m_szHeap / 4�Ķ���ֵ
				size_t szGet = 2 * szTotal + ROUNDUP(heap_size_ >> 4);

				// ����Heap�ռ䣬���������ڴ��
				start_free_ = reinterpret_cast<char *>(AllocType::allocate(szGet));

				if( NULL == start_free_ )
				{
					// û�з��䵽�ڴ棬ת��MallocMemoryPool
					end_free_ = 0;
					start_free_ = reinterpret_cast<char *>(malloc_pool::allocate(szGet));
					if( NULL == start_free_ )
						throw std::bad_alloc();
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
				AllocType::deallocate(iter->first, iter->second);
			}
			buffers_.clear();
		}
	};

}




#endif