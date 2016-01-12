#ifndef __CONTAINER_SYNC_CONTAINER_HPP
#define __CONTAINER_SYNC_CONTAINER_HPP

/** @async_container.hpp
*
* @author <����>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* �̰߳�ȫ��������,�ṩ���������͹�������
*/


#include <vector>
#include <map>
#include <mutex>

/*
�̰߳�ȫ��������(vector, list, deque)
sync_sequence_container_t

�̰߳�ȫ��������(map, set, multimap, multiset, hash)
sync_assoc_container_t

*/


namespace stdex
{


	namespace container
	{
		/**
		* @class <sync_sequence_container_t>
		* @brief �����������̰߳�ȫ���ӿ���stl��������
		*
		* T ֵ����
		* C �������ͣ�����vector��list��deque��Ĭ��ʹ��vector
		* S ͬ�����ͣ�����critical_section��event_t��mutex
		*/

		template < 
			typename T,
			typename C = std::vector<T>, 
			typename S = std::mutex
		>
		class sync_sequence_container_t
		{
		public:
			typedef S Mutex;
			typedef std::unique_lock<Mutex>	AutoLock;
			typedef C Container;

			typedef typename Container::value_type		value_type;
			typedef typename Container::iterator		iterator;
			typedef typename Container::const_iterator	const_iterator;
			typedef typename Container::reference		reference;
			typedef typename Container::const_reference	const_reference;
			typedef typename Container::allocator_type	allocator_type;

		private:
			mutable Mutex mutex_;
			Container container_;

		public:
			sync_sequence_container_t()
			{}

			/**
			* @brief ����һ��allocator
			* @param <alloc> <allocator����>
			* @exception <�����׳��κ��쳣>
			* @return <��>
			* @note <���ݴ����������ͣ����������ڴ������>
			* @remarks <����ڴ����Ч��>
			*/
			explicit sync_sequence_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

		private:
			sync_sequence_container_t(const sync_sequence_container_t &);
			sync_sequence_container_t &operator=(const sync_sequence_container_t &);

		public:
			iterator begin()
			{ 
				AutoLock lock(mutex_);
				return container_.begin();
			}
			const_iterator begin() const
			{ 
				AutoLock lock(mutex_);
				return container_.begin();
			}
			iterator end()
			{ 
				AutoLock lock(mutex_);
				return container_.end();
			}
			const_iterator end() const
			{ 
				AutoLock lock(mutex_);
				return container_.end();
			}

			size_t size() const
			{
				AutoLock lock(mutex_);
				return container_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return container_.empty();
			}

			void clear()
			{
				AutoLock lock(mutex_);
				container_.clear();
			}

			reference operator[](size_t pos)
			{
				AutoLock lock(mutex_);
				iterator iter = container_.begin();
				std::advance(iter, pos);

				return *iter;
			}

			const_reference top() const
			{
				AutoLock lock(mutex_);
				return container_.front();
			}

			void pop_top()
			{
				AutoLock lock(mutex_);
				container_.pop_front();
			}

			void push_front(const T &val)
			{
				AutoLock lock(mutex_);
				return container_.push_front(val);
			}
			void push_back(const T &val)
			{
				AutoLock lock(mutex_);
				return container_.push_back(val);
			}

			/**
			* @brief ������������
			* @param <op> <�ص���������>
			* @exception <�����׳��κ��쳣>
			* @return <��>
			* @note <op����Ϊfunction<void(const T &)>,����һ�������Ļص�����>
			* @remarks <>
			*/
			template < typename OP >
			void for_each(const OP &op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			/**
			* @brief �������func��������ִ��op
			* @param <func> <�����ص�����������bool������һ��const T &����>
			* @param <op> <�ص���������������һ��const T &����>
			* @exception <�����׳��κ��쳣>
			* @return <����һ��������>
			* @note <���ͨ��functor��⣬��ִ��op>
			* @remarks <ʹ��find_if>
			*/
			template < typename Functor, typename OP >
			iterator op_if(const Functor &func, const OP &op)
			{
				AutoLock lock(mutex_);
				iterator iter = find_if(func);
				if( iter != container_.end() )
					op(*iter);

				return iter;
			}

			/**
			* @brief �������func������ִ��op1������ִ��op2
			* @param <func> <�����ص�����������bool������һ��const T &����>
			* @param <op1> <�ص���������������һ��const T &����>
			* @param <op2> <�ص���������������һ��const T &����>
			* @exception <�����׳��κ��쳣>
			* @return <��>
			* @note <��>
			* @remarks <ʹ��find_if>
			*/
			template < typename Functor, typename OP1, typename OP2 >
			void op_if(const Functor &func, const OP1 &op1, const OP2 &op2)
			{
				AutoLock lock(mutex_);
				iterator iter = std::find_if(container_.begin(), container_.end(), func);
				if( iter != container_.end() )
					op1(*iter);
				else
					op2();
			}

			/**
			* @brief ��������������Ԫ�ص�һ����������op
			* @param <func> <�����ص�����������bool������һ��const T &����>
			* @param <op> <�ص���������������һ��const T &����������ֵΪbool>
			* @exception <�����׳��κ��쳣>
			* @return <���ص����������û������������Ԫ�أ��򷵻�һ����Ч�����������򷵻ص�һ������������Ԫ�ص�����>
			* @note <���ص�һ�����������ĵ�����>
			* @remarks <��>
			*/
			template < typename OP >
			iterator find_if(const OP &op)
			{
				AutoLock lock(mutex_);
				return std::find_if(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			const_iterator find_if(const OP &op) const
			{
				AutoLock lock(mutex_);
				return std::find_if(container_.begin(), container_.end(), op);
			}

			/**
			* @brief ɾ��������Ԫ�ص�һ����������op��Ԫ��
			* @param <op> <�ص���������������һ��const T &����������ֵΪbool>
			* @exception <�����׳��κ��쳣>
			* @return <��>
			* @note <���û������������Ԫ�أ��򲻽���ɾ��>
			* @remarks <ʹ��find_if>
			*/
			template < typename OP >
			void erase(const OP &op)
			{
				AutoLock lock(mutex_);
				const_iterator iter = find_if(op);
				if( iter != container_.end() )
					container_.erase(iter);
			}

			void sort()
			{
				AutoLock lock(mutex_);
				std::sort(container_.begin(), container_.end());
			}

			/**
			* @brief ����������Ԫ�ؽ�������
			* @param <op> <�ص���������������һ��const T &����������ֵΪbool����Ҫ֧��'<' >
			* @exception <�����׳��κ��쳣>
			* @return <��>
			* @note <op��Ҫ֧��'<'>
			* @remarks <��>
			*/
			template < typename OP >
			void sort(const OP &op)
			{
				AutoLock lock(mutex_);
				std::sort(container_.begin(), container_.end(), op);
			}

		};



		/**
		* @class <sync_assoc_container_t>
		* @brief �����������̰߳�ȫ���ӿ���stl��������
		*
		* K key����
		* V value����
		* C �������ͣ�����map��set��multi_map��multi_set��unordered_map��unordered_set
		* S ͬ�����ͣ�����critical_section��event_t��mutex
		*/

		template < 
			typename K, 
			typename V,
			typename C = std::map<K, V>, 
			typename S = std::mutex
		>
		class sync_assoc_container_t
		{
		public:
			typedef S Mutex;
			typedef std::unique_lock<Mutex>	AutoLock;
			typedef C Container;

			typedef typename Container::key_type		key_type;
			typedef typename Container::mapped_type		mapped_type;
			typedef typename Container::value_type		value_type;
			typedef typename Container::iterator		iterator;
			typedef typename Container::const_iterator	const_iterator;
			typedef typename Container::reference		reference;
			typedef typename Container::const_reference	const_reference;
			typedef typename Container::allocator_type	allocator_type;

		private:
			mutable Mutex mutex_;
			Container container_;

		public:
			sync_assoc_container_t()
			{}

			/**
			* @brief ����һ��allocator
			* @param <alloc> <allocator����>
			* @exception <�����׳��κ��쳣>
			* @return <��>
			* @note <���ݴ����������ͣ����������ڴ������>
			* @remarks <����ڴ����Ч��>
			*/
			explicit sync_assoc_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

			sync_assoc_container_t(const sync_assoc_container_t &) = delete;
			sync_assoc_container_t &operator=(const sync_assoc_container_t &) = delete;

		public:
			size_t size() const
			{
				AutoLock lock(mutex_);
				return container_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return container_.empty();
			}

			void clear()
			{
				AutoLock lock(mutex_);
				container_.clear();
			}

			mapped_type &operator[](const key_type &key)
			{
				AutoLock lock(mutex_);
				return container_[key];
			}

			const mapped_type &operator[](const key_type &key) const
			{
				AutoLock lock(mutex_);
				return container_.at(key);
			}

			bool exsit(const key_type &key) const
			{
				AutoLock lock(mutex_);
				return container_.find(key) != container_.end();
			}

			void insert(key_type &&key, mapped_type &&val)
			{
				AutoLock lock(mutex_);
				container_.emplace(std::move(key), std::move(val));
			}

			void insert(const key_type &key, const mapped_type &val)
			{
				AutoLock lock(mutex_);
				container_.insert(std::make_pair(key, val));
			}

			template < typename OP >
			void for_each(const OP &op) const
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			void for_each(OP op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

		
			/**
			* @brief ������������Ҳ���key����ִ��op
			* @param <key> <key�ؼ���>
			* @param <op> <�ص���������������һ��const value_type &����>
			* @exception <�����׳��κ��쳣>
			* @return <���ִ��op���򷵻�true�����򷵻�false>
			* @note <��>
			* @remarks <��>
			*/
			template < typename OP >
			bool not_if_op(const key_type &key, OP &&op)
			{
				AutoLock lock(mutex_);
				const_iterator iter = container_.find(key);
				if( iter == container_.end() )
				{
					op();
					return true;
				}

				return false;
			}

			/**
			* @brief ������������ҵ�key����ִ��op
			* @param <key> <key�ؼ���>
			* @param <op> <�ص���������������һ��const value_type &����>
			* @exception <�����׳��κ��쳣>
			* @return <���ִ��op���򷵻�true�����򷵻�false>
			* @note <��>
			* @remarks <��>
			*/
			template < typename OP >
			bool op_if(const key_type &key, OP op)
			{
				AutoLock lock(mutex_);
				iterator iter = container_.find(key);
				if( iter != container_.end() )
				{
					op(*iter);
					return true;
				}

				return false;
			}

			void erase(const key_type &key)
			{
				AutoLock lock(mutex_);

				container_.erase(key);
			}
		};

	}

}



#endif