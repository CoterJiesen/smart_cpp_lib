#ifndef __UTILITY_SCOPE_EXIT_HPP
#define __UTILITY_SCOPE_EXIT_HPP

/** @ini.hpp
*
* @author <����>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* ����RAII���ƣ��˳����������ָ������
*/

#include <functional>
#include <memory>


namespace utility
{

	/**
	* @class <scope_exit_t>
	* @brief 
	*
	* ����C++RAII���ƣ�����ʱִ��InitFunct������ʱִ��UninitFuncT
	*/
	template < typename InitFuncT, typename UninitFuncT >
	class scope_exit_t
	{
		typedef InitFuncT		InitFuncionType;
		typedef UninitFuncT		UninitFunctionType;

	private:
		InitFuncionType			initFunc_;		// ��ʼ��
		UninitFunctionType		unInitFunc_;	// ����ʼ��

	public:
		/**
		* @brief ���캯������Ҫ�����ʼ��ִ�й��̺ͷ���ʼ����ִ�й���
		* @param <init> <��ʼ��ִ�й���>
		* @param <uninit> <����ʼ��ִ�й���>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		scope_exit_t(const InitFuncionType &init, const UninitFunctionType &unInt)
			: initFunc_(init)
			, unInitFunc_(unInt)
		{
			initFunc_();
		}
		~scope_exit_t()
		{
			unInitFunc_();
		}

		scope_exit_t(scope_exit_t &&rhs)
			: initFunc_(std::move(rhs.initFunc_))
			, unInitFunc_(std::move(rhs.unInitFunc_))
		{}

		scope_exit_t &operator=(scope_exit_t &&rhs)
		{
			if( &rhs != this )
			{
				initFunc_ = std::move(rhs.initFunc_);
				unInitFunc_ = std::move(rhs.unInitFunc_);
			}
		}

	private:
		scope_exit_t(const scope_exit_t &);
		scope_exit_t &operator=(const scope_exit_t &);
	};


	/**
	* @brief���ñ����������Ƶ�����scope_exit����
	* @param <init> <��ʼ��ִ�й���>
	* @param <uninit> <����ʼ��ִ�й���>
	* @exception <���κ��쳣�׳�>
	* @return <��>
	* @note <��>
	* @remarks <��>
	*/
	template < typename InitT, typename UninitT >
	inline scope_exit_t<InitT, UninitT> make_scope_exit(const InitT &init, const UninitT &uninit)
	{
		return scope_exit_t<InitT, UninitT>(init, uninit);
	}
}




#endif