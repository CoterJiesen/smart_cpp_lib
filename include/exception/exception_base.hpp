#ifndef __EXCEPTION_BASE_HPP
#define __EXCEPTION_BASE_HPP


/** @exception_base.hpp
*
* @author <����>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* �쳣�����࣬
*	debugģʽ�£�������׳��쳣�ĵ��ö�ջ
*	releaseģʽ�£��򲻻������ջ
* �����ͨ�����Դ��ڲ鿴��ջ��Ϣ
*/

#include <windows.h>
#include <exception>
#include <sstream>
#include <string>
#include <type_traits>
#include <system_error>

#include "../extend_stl/string/algorithm.hpp"	// for stdex::to_string
#include "../win32/debug/stack_walker.hpp"		// for win32::debug::dump_stack


namespace exception
{
	// ---------------------------------

	namespace detail
	{
		class dump_helper
		{
			std::ostringstream dump_;

		public:
			dump_helper()
			{
				win32::debug::dump_stack(
					[this](void *pvAddress, size_t lineNum, const char * fileName, const char * szModule, const char * szSymbol)
				{
					dump_ << fileName << "(" << lineNum << "): " <<  szSymbol << std::endl;
				}, 
					[](const char *msg)
				{
					assert(0);
				}, 
					5);	// offset frames number
			}

			dump_helper(dump_helper &rhs)
			{
				dump_.swap(rhs.dump_);
			}

		public:
			void dump() const
			{
				::OutputDebugStringA(dump_.str().c_str());
			}
		};

		
		struct dump_null
		{
			void dump() const
			{}
		};
	}
	

	/**
	* @class <exception_base_t>
	* @brief �쳣�����࣬���׳��쳣ʱ�ڵ��Դ��ڴ�ӡ��ջ��Ϣ
	*
	* DumpT dump policy
	*	detail::dump_helper	debugģʽʹ��
	*	detail::dump_null	releaseģʽʹ��
	*/

	template < typename DumpT >
	class exception_base_t
		: public std::exception
		, private DumpT
	{
	protected:
		std::error_code code_;
		std::string msg_;

	public:
		exception_base_t(std::error_code code, const std::string &msg)
			: msg_(msg)
			, code_(code)
		{}
		virtual ~exception_base_t()
		{

		}

	protected:
		exception_base_t(exception_base_t &rhs)
		{	
			msg_.swap(rhs.msg_);
		}

		exception_base_t &operator=(const exception_base_t &);
		

	public:
		/**
		* @brief ׷�Ӳ�����Ϣ
		* @param <val> <���ͻ򸡵�����ֵ>
		* @exception <�����׳��κ��쳣>
		* @return <exception_base_t &>
		* @note <����val����Ϊ���ͻ򸡵�����ֵ>
		* @remarks <>
		*/
		template < typename T >
		exception_base_t &operator<<(const T &val)
		{
			static_assert(std::is_integral<T>::value || 
				std::is_floating_point<T>::value, "T must be a number or char");

			std::string &&tmp = stdex::to_string(val);
			msg_.append(tmp);

			return *this;
		}

		/**
		* @brief ׷�Ӳ�����Ϣ
		* @param <val> <ascii�ַ���>
		* @exception <�����׳��κ��쳣>
		* @return <exception_base_t &>
		* @note <>
		* @remarks <>
		*/
		exception_base_t &operator<<(const char *val)
		{
			msg_.append(val, ::strlen(val));

			return *this;
		}

		/**
		* @brief ׷�Ӳ�����Ϣ
		* @param <val> <std::string�ַ���>
		* @exception <�����׳��κ��쳣>
		* @return <exception_base_t &>
		* @note <>
		* @remarks <>
		*/
		exception_base_t &operator<<(const std::string &val)
		{
			msg_.append(val);

			return *this;
		}

		/**
		* @brief �����쳣���ٵ��Դ��ڴ�ӡ��ջ��Ϣ
		* @param <>
		* @exception <�����׳��κ��쳣>
		* @return <>
		* @note <�ڲ����쳣�����>
		* @remarks <>
		*/
		void dump() const
		{
			DumpT::dump();
		}

	public:
		virtual const char *what() const
		{
			return msg_.c_str();
		}

		std::error_code code() const
		{
			return code_;
		}
	};


#ifdef _DEBUG
	typedef exception_base_t<detail::dump_helper> exception_base;
#else
	typedef exception_base_t<detail::dump_null>	exception_base;
#endif
}




#endif