#ifndef __UTILITY_INI_HPP
#define __UTILITY_INI_HPP

/** @ini.hpp
*
* @author <����>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* ini�ļ����ʣ�֧��ascii��unicode�ַ���
*/


#include <unordered_map>
#include <string>

#include "../extend_stl/unicode.hpp"
#include "../extend_stl/string/algorithm.hpp"


namespace utility
{
	/**
	* @class <ini>
	* @brief �߳���ؾֲ��洢
	*
	* ���̰߳�ȫ
	*/
	class ini
	{
		struct ret_helper
		{
			const stdex::tString &buffer_;

			ret_helper(const stdex::tString &buffer)
				: buffer_(buffer)
			{}

			template < typename T >
			operator T()
			{
				return stdex::to_number(buffer_);
			}

			operator const stdex::tString &() const
			{
				return buffer_;
			}
		};

	public:
		typedef stdex::tString section_type;
		typedef stdex::tString key_type;
		typedef stdex::tString value_type;
		
	private:
		typedef std::unordered_map<key_type, value_type> key_value_map_type;
		typedef std::unordered_map<section_type, key_value_map_type> ini_map_type;

		ini_map_type ini_maps_;
		
	public:
		/**
		* @brief ���캯������Ҫ����ini�ļ�����·��
		* @param <path> <ini�ļ�����·��>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <����ļ������ڣ��򲻽��н���>
		* @remarks <��>
		*/
		explicit ini(const stdex::tString &path);

		/**
		* @brief ���캯������Ҫ�����ڴ滺�����ͻ�������С
		* @param <path> <ini�ļ�����·��>
		* @exception <���κ��쳣�׳�>
		* @return <��>
		* @note <��>
		* @remarks <��>
		*/
		ini(const char *buffer, size_t len);

	private:
		ini(const ini &);
		ini &operator=(const ini &);

	public:
		/**
		* @brief ��ȡָ��section��ָ��key��ֵ
		* @param <section> <section��>
		* @param <key> <��section����key�ؼ���>
		* @exception <���κ��쳣�׳�>
		* @return <���ݽ��������ͣ���ʽת����ֵ>
		* @note <��>
		* @remarks <��>
		*/
		ret_helper get_val(const section_type &section, const key_type &key) const;

		/**
		* @brief ��ȡ����section
		* @param <��>
		* @param <��>
		* @exception <���κ��쳣�׳�>
		* @return <��������section>
		* @note <��>
		* @remarks <��>
		*/
		std::vector<stdex::tString> get_all_sections() const;
	};
}


#endif