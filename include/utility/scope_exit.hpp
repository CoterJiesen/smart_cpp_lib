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
	struct deletor_t
	{
		template < typename T >
		void operator()(T *p)
		{
			(*p)();
		}
	};
	
	template < typename D >
	auto make_scope_exit(D &&d)->std::unique_ptr<D, deletor_t>
	{
		return std::unique_ptr<D, deletor_t>(&d, deletor_t());
	}
}




#endif