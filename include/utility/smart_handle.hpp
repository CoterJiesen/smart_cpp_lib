#ifndef __UTILITY_SMART_HANDLE_HPP
#define __UTILITY_SMART_HANDLE_HPP

/** @smart_handle.hpp
*
* @author <����>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* ����RAII���ƣ��˳�������ʱ�رվ��
*/

#include <Windows.h>
#include <Winsvc.h>
#include <type_traits>
#include <memory>



namespace utility
{

	/**
	* @class <smart_handle_t>
	* @brief 
	*
	* ����C++RAII���ƣ��˳�������ʱ�ر��Ѵ򿪾������Դ�Զ�����
	*/
	template <
		typename HandleT, 
		template< typename > class ReleasePolicyT, 
		HandleT NULL_VALUE = NULL
	>
	class smart_handle_t 
		: private ReleasePolicyT<HandleT>
	{
		typedef HandleT	value_type;

	private:
		value_type handle_;

	public:
		/**
		* @brief Ĭ�Ϲ��캯������ʼ�����Ϊ��
		* @param <��>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		smart_handle_t()
			: handle_(NULL_VALUE)
		{
		}

		/**
		* @brief ����һ������Ĺ��캯��
		* @param <h> <windows���>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		smart_handle_t(const value_type &h)
			: handle_(h)
		{
		}

		/**
		* @brief �������캯��
		* @param <h> <smart_handle_t����>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		smart_handle_t(const smart_handle_t &h)
		{
			cleanup();
			handle_ = h.handle_;
		}

		~smart_handle_t()
		{
			cleanup();
		}

		smart_handle_t &operator=(const smart_handle_t &rhs) 
		{ 
			if( &rhs != this )
			{
				cleanup();
				handle_ = rhs.handle_;
			}

			return(*this);  
		}

		value_type &operator=(const value_type &hande) 
		{ 
			if( hande != handle_ )
			{
				cleanup();
				handle_ = hande;
			}

			return handle_;  
		}

		/**
		* @brief ��ʽת������������һ��windows���
		* @param <��>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		operator value_type &()
		{
			return handle_;
		}

		operator const value_type &() const
		{
			return handle_;
		}

		/**
		* @brief ��ʾ��ȡһ��windows���
		* @param <��>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		value_type &get()
		{
			return handle_;
		}

		const value_type &get() const
		{
			return handle_;
		}

		/**
		* @brief ��ʾ�жϸþ���Ƿ���Ч
		* @param <��>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		bool is_valid() const
		{
			return handle_ != NULL_VALUE;
		}

		/**
		* @brief ��ʽ�жϸþ���Ƿ���Ч
		* @param <��>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		operator bool() const
		{
			return is_valid();
		}

		/**
		* @brief ����������������Զ�����
		* @param <��>
		* @exception <���κ��쳣�׳�>
		* @return <����windows���>
		* @note <��>
		* @remarks <��>
		*/
		value_type detach()
		{
			value_type hHandle = handle_;
			handle_ = NULL_VALUE;

			return hHandle;
		}

		void cleanup()
		{
			if ( handle_ != NULL_VALUE )
			{
				operator()(handle_);
				handle_ = NULL_VALUE;
			}
		}
	};


	namespace detail
	{
		// Release algorithms (release policies)

		template< typename T >
		struct close_handle_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::CloseHandle(handle);
				assert(suc);
			}
		};



		template < typename T >
		struct close_reg_t
		{
			void operator()(T handle)
			{
				LONG ret = ::RegCloseKey(handle);
				assert(ERROR_SUCCESS == ret);
			}
		};


		template < typename T >
		struct close_libaray_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::FreeLibrary(handle);
				assert(suc);
			}
		};


		template < typename T >
		struct close_mmap_file_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::UnmapViewOfFile(handle);
				assert(suc);
			}
		};


		template < typename T >
		struct close_service_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::CloseServiceHandle(handle);
				assert(suc);
			}
		};

		template < typename T >
		struct close_find_file_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::FindClose(handle);
				assert(suc);
			}
		};

		template < typename T >
		struct close_icon_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::DestroyIcon(handle);
				assert(suc);
			}
		};

	}


	// definitions of standard Windows handles
	
	typedef smart_handle_t<HANDLE,	detail::close_handle_t>		                    auto_handle;
	typedef smart_handle_t<HKEY,	detail::close_reg_t>		                    auto_reg;
	typedef smart_handle_t<PVOID,	detail::close_mmap_file_t>	                    auto_mmap_file;
	typedef smart_handle_t<HMODULE,	detail::close_libaray_t>						auto_libaray;
	typedef smart_handle_t<HANDLE,	detail::close_handle_t,		INVALID_HANDLE_VALUE>	auto_file;
	typedef smart_handle_t<SC_HANDLE, detail::close_service_t>						auto_service;
	typedef smart_handle_t<HANDLE,	detail::close_find_file_t,	INVALID_HANDLE_VALUE> auto_find_file;
	typedef smart_handle_t<HICON,	detail::close_icon_t>							auto_icon;
	typedef smart_handle_t<HANDLE,	detail::close_handle_t,		INVALID_HANDLE_VALUE>	auto_tool_help;
	typedef smart_handle_t<HANDLE,	detail::close_handle_t>							auto_token;

	

}


#endif