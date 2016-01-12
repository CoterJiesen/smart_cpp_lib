#ifndef __MULTI_THREAD_TLS_HPP
#define __MULTI_THREAD_TLS_HPP

#include <exception>
#include <stdexcept>
#include <windows.h>
#include <cassert>

namespace multi_thread
{
	/**
	* @class <tls_ptr_t>
	* @brief �߳���ؾֲ��洢
	*
	* T �洢����������
	*/
	template < typename T >
	class tls_ptr_t
	{
		typedef T value_type;

	private:
		// ����߳��Ƿ����̳߳���
		DWORD tss_key_;

	public:
		/**
		* @brief �߳���ؾֲ��洢Ĭ�Ϲ��캯��
		* @param <��>
		* @exception <�������ؼ���ʧ�ܣ����׳�runtime_error�쳣>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		tls_ptr_t()
			: tss_key_(::TlsAlloc())
		{
			if( tss_key_ == TLS_OUT_OF_INDEXES )
				throw std::runtime_error("TlsAlloc");
		}
		explicit tls_ptr_t(value_type *val)
			: tss_key_(::TlsAlloc())
		{
			if( tss_key_ == TLS_OUT_OF_INDEXES )
				throw std::runtime_error("TlsAlloc");

			::TlsSetValue(tss_key_, val);
		}

		~tls_ptr_t()
		{
			BOOL suc = ::TlsFree(tss_key_);
			assert(suc);
		}

	public:
		value_type *get()
		{
			return static_cast<value_type *>(::TlsGetValue(tss_key_));
		}

		value_type *operator->()
		{
			return static_cast<value_type *>(::TlsGetValue(tss_key_));
		}

		void operator=(value_type *val)
		{
			assert(::TlsGetValue(tss_key_) == nullptr);
			::TlsSetValue(tss_key_, val);
		}

		explicit operator bool() const
		{
			return ::TlsGetValue(tss_key_) != nullptr;
		}
	};



	// --------------------------------------------------
	// class CallStack

	// ��⵱ǰ�Ƿ����߳̽��з���

	template < typename OwnerT >
	class call_stack_t
	{
	public:
		// ��ջ������owner
		class context;

	private:
		// ��ջ���ĵ���
		static tls_ptr_t<context> top_;

	public:
		// ���owner�Ƿ���ջ��
		static bool contains(OwnerT *owner)
		{
			context *val = top_;
			while( val )
			{
				if( val->owner_ == owner )
					return true;

				val = val->next_;
			}

			return false;
		}

	};

	template < typename OwnerT >
	tls_ptr_t<typename call_stack_t<OwnerT>::context> call_stack_t<OwnerT>::top_;



	template < typename OwnerT >
	class call_stack_t<OwnerT>::context
	{
	private:
		OwnerT *owner_;		// owner��context����
		context *next_;		// ��ջ�ϵ���һ��Ԫ��

		friend class call_stack_t<OwnerT>;

	public:
		explicit context(OwnerT *owner)
			: owner_(owner)
			, next_(call_stack_t<OwnerT>::top_)
		{
			call_stack_t<OwnerT>::top_ = this;
		}
		~context()
		{
			call_stack_t<OwnerT>::top_ = next_;
		}

	private:
		context(const context &);
		context &operator=(const context &);
	};
}






#endif