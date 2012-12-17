#ifndef __MULTI_THREAD_THREADPOOL_IMPL_HPP
#define __MULTI_THREAD_THREADPOOL_IMPL_HPP



#include <vector>

#include "thread.hpp"
#include "../task_adaptor.hpp"
#include "../../../extend_stl/atomic.hpp"


namespace multi_thread
{
	namespace threadpool
	{
		namespace detail
		{

			// TaskT policies:		TaskFunction, PrioTaskFunc, 
			// ScheduleT policies:	FifoScheduler, LifoScheduler, PrioScheduler

			// ---------------------------------------

			template < 
				typename TaskT,
				template < typename > class ScheduleT,
				template < typename > class SizeT
			>
			class thread_pool_impl_t
			{
			public:
				typedef thread_pool_impl_t<TaskT, ScheduleT, SizeT>	this_type;

				typedef TaskT									task_type;
				typedef ScheduleT<task_type>					scheduler_type;
				typedef SizeT<this_type>						size_ctrl_type;

				typedef worker_thread_t<this_type>				work_thread_type;
				typedef std::shared_ptr<work_thread_type>		work_thread_ptr;
				typedef std::vector<work_thread_ptr>			work_threads_type;

			private:
				friend class work_thread_type;

				scheduler_type scheduler_;							// ���Ȳ���
				size_ctrl_type sizeCtl_;							// �߳�������
				work_threads_type work_threads_;					// �߳���
			
				stdex::atomic<size_t>	active_num_;


			public:
				thread_pool_impl_t()
				{
					active_num_ = 0;
				}
				~thread_pool_impl_t()
				{
				}

			private:
				thread_pool_impl_t(const thread_pool_impl_t &);
				thread_pool_impl_t &operator=(const thread_pool_impl_t &);

			public:
				// ��ȡ�̸߳���
				size_t size() const
				{
					return work_threads_.size();
				}

				// ��ȡ����ִ��������̸߳���
				size_t active_size() const
				{
					return active_num_;
				}


				// ����һ��
				void shutdown()
				{
					terminate();
				}

				// ����
				void schedule(const task_type &task)
				{
					// ��������
					if( active_num_ == size() )
					{
						sizeCtl_.resize(*this, size() + 1);
					}

					scheduler_.push(task);
				}

				// ���
				void clear()
				{
					scheduler_.clear();
				}

				// �Ƿ�Ϊ��
				bool empty() const
				{
					return scheduler_.empty();
				}


				// �ر����й����߳�
				void terminate()
				{
					for(size_t i = 0; i != work_threads_.size(); ++i)
					{
						scheduler_.push(task_type());
					}
					
					work_threads_.clear();
				}

				// ���������̸߳���
				bool resize(size_t workCnt)
				{
					while( work_threads_.size() < workCnt )
					{
						work_thread_ptr worker =  work_thread_type::create(this);
						work_threads_.push_back(worker);
					}
					
					if( work_threads_.size() > workCnt )
					{
						work_threads_.resize(workCnt);
					}

					return true;
				}

				// �ӷǿ������л�ȡ����ִ��
				bool exceute_task()
				{
					task_type task = scheduler_.pop();
					if( task )
					{
						--active_num_;
						task();
						++active_num_;

						return true;
					}
					else
						return false;
				}

			};
		}
	}
}






#endif