#ifndef __NETWORK_HELPER_HPP
#define __NETWORK_HELPER_HPP


/** @network_helper.hpp
*
* @author <����>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* win32��������ذ�������
*/


#include <cstdint>

#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <vector>
#include <Psapi.h>


#include "../../extend_stl/string/algorithm.hpp"
#include "../../unicode/string.hpp"
#include "../../utility/select.hpp"


#pragma comment(lib, "Psapi")
#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "Iphlpapi")


/*
ipת��
	ip_2_string
	string_2_ip

��֤IP
	is_valid_ip	

���port�Ƿ�ռ��
	is_port_used
	
����������Ϣ
	get_local_ips
	get_local_gate_ips
	get_mac_addr
	get_local_dns

macת��
	mac_string_to_binary

�Ƿ񱾵��ж�
	is_local_by_ip
	is_local_by_machine_name


����socket��ȡIP
	get_sck_ip

*/


namespace win32
{
	namespace network
	{

		namespace detail
		{
			struct ip_2_string_helper_t
			{
				unsigned long ip_;
				ip_2_string_helper_t(unsigned long ip)
					:ip_(ip)
				{}

				template < typename CharT >
				operator std::basic_string<CharT>() const
				{
					std::basic_ostringstream<CharT> os_;

					in_addr tmp = {0};
					tmp.s_addr = ip_;

					char *p = ::inet_ntoa(tmp);
					os_ << p;
					
					return std::move(os_.str());
				}
			};
		}

		/**
		* @brief IPת�ַ���
		* @param <ip> <IP���ݣ�������>
		* @exception <���κ��쳣�׳�>
		* @return <����IP��Ӧ���ַ���>
		* @note <��>
		* @remarks <��>
		*/
		inline detail::ip_2_string_helper_t ip_2_string(unsigned long ip)
		{
			return detail::ip_2_string_helper_t(ip);
		}
		
		/**
		* @brief �ַ���תIP
		* @param <ip> <IP�ַ���>
		* @exception <���κ��쳣�׳�>
		* @return <����IP�� ������>
		* @note <���ص���������IP����>
		* @remarks <��>
		*/
		inline unsigned long string_2_ip(const std::string &ip)
		{
			return ::ntohl(::inet_addr(ip.c_str()));
		}

		inline unsigned long string_2_ip(const std::wstring &ip)
		{
			return string_2_ip((LPCSTR)CW2A(ip.c_str()));
		}


		/**
		* @brief �ж��Ƿ�Ϊ�Ϸ�IP
		* @param <ip> <IP�ַ���>
		* @exception <���κ��쳣�׳�>
		* @return <���Ϊ�Ϸ�IP��Ϊtrue������Ϊfalse>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		inline bool is_valid_ip(const std::basic_string<CharT> &ip)
		{
			int nIP = 0, nCount = 0;   

			std::vector<std::basic_string<CharT>> vecIP;
			stdex::split(vecIP, ip, CharT('.'));

			// ��4���� ��Ϊ�����Ϲ���
			if( vecIP.size() != 4 ) 
				return false;

			// ip��β��Ϊ0 ��Ϊ�����Ϲ���
			if( *(vecIP.begin()) == utility::select<CharT>("0", L"0") || 
				*(vecIP.rbegin()) == utility::select<CharT>("0", L"0") )
				return false;

			for(auto iter = vecIP.begin(); iter != vecIP.end(); ++iter)
			{
				if( *iter == utility::select<CharT>("255", L"255") ) 
					++nCount;

				// ip ÿ���β���0��255��Χ�� ��Ϊ�����Ϲ���
				nIP = stdex::to_number(*iter);
				if ( nIP < 0 || nIP > 255 )
					return false;
			}

			// ip ÿ���ζ���255 ��Ϊ�����Ϲ���
			if ( nCount == 4 ) 
				return false;

			// ip ���ͳɹ� ��Ϊ���Ϲ���
			if( ::inet_addr(unicode::translate_t<CharT>::utf(ip).c_str()) != INADDR_NONE )   
				return true; 

			// ������Ϊ�����Ϲ���
			return false;
		}

		template < typename CharT >
		inline bool is_valid_ip(const CharT *ip)
		{
			std::basic_string<CharT> val(ip);
			return is_valid_ip(val);
		}

		/**
		* @brief �õ���������IP
		* @param <IPs> <IP�ַ�������>
		* @exception <���κ��쳣�׳�>
		* @return <����ɹ�����䱾���ϵ�IP>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		inline bool get_local_ips(std::vector<std::basic_string<CharT>> &IPs)
		{
			IP_ADAPTER_INFO info[16] = {0};
			DWORD dwSize = sizeof(info);
			if( ERROR_SUCCESS != ::GetAdaptersInfo(info, &dwSize) )
				return false; 

			PIP_ADAPTER_INFO pAdapter = info;
			while (pAdapter != NULL)
			{
				PIP_ADDR_STRING pAddr = &pAdapter->IpAddressList;
				while (pAddr != NULL)
				{
					std::basic_string<CharT> tmp = unicode::translate_t<CharT>::utf(pAddr->IpAddress.String);

					IPs.push_back(std::move(tmp));
					pAddr = pAddr->Next;
				}
				pAdapter = pAdapter->Next;
			}
			return true;
		}


		/**
		* @brief ��ȡ��������IP
		* @param <IPs> <IP�ַ�������>
		* @exception <���κ��쳣�׳�>
		* @return <����ɹ�����䱾���ϵ�����IP>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		inline bool get_local_gate_ips(std::vector<std::basic_string<CharT>> &gateIPs)
		{
			IP_ADAPTER_INFO info[16] = {0};
			DWORD dwSize = sizeof(info);
			if( ERROR_SUCCESS != ::GetAdaptersInfo(info, &dwSize) )
				return false; 

			PIP_ADAPTER_INFO pAdapter = info;
			while (pAdapter != NULL)
			{
				PIP_ADDR_STRING pAddr = &pAdapter->GatewayList;
				while (pAddr != NULL)
				{
					std::basic_string<CharT> &&tmp = unicode::to(std::basic_string<CharT>(pAddr->IpAddress.String));
					gateIPs.push_back(std::move(tmp));
					pAddr = pAddr->Next;
				}
				pAdapter = pAdapter->Next;
			}
			return true;
		}

		/**
		* @brief ����IP��ȡMAC��ַ
		* @param <ip> <ip�ַ���>
		* @exception <���κ��쳣�׳�>
		* @return <����ɹ��򷵻ظ�IP��ӦMAC��ַ>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		inline std::basic_string<CharT> get_mac_addr(const std::basic_string<CharT> &ip)
		{
			assert(is_valid_ip(ip));

			const int MAX_ADAPTER_NUM = 10; //���֧��10������
			IP_ADAPTER_INFO astAdapter[MAX_ADAPTER_NUM] = {0};
			ULONG nSize = sizeof(astAdapter);
			if( ERROR_SUCCESS != ::GetAdaptersInfo(astAdapter, &nSize) )
			{
				assert(0 && "��������������Ԥ��");
				return std::basic_string<CharT>();
			}

			const std::string &&srcIP = unicode::to(ip);
			for(PIP_ADAPTER_INFO pAdapter = astAdapter; pAdapter != NULL; pAdapter = pAdapter->Next)
			{
				// ȷ������̫��,ȷ��MAC��ַ�ĳ���Ϊ 00-00-00-00-00-00
				if(pAdapter->Type == MIB_IF_TYPE_ETHERNET && 
					pAdapter->AddressLength == 6 && 
					srcIP == pAdapter->IpAddressList.IpAddress.String)
				{
					CharT mac[32] = {0};
					utility::select<CharT>(sprintf_s, swprintf_s)(mac, _countof(mac), 
						utility::select<CharT>("%02X-%02X-%02X-%02X-%02X-%02X", L"%02X-%02X-%02X-%02X-%02X-%02X"),
						int (pAdapter->Address[0]),
						int (pAdapter->Address[1]),
						int (pAdapter->Address[2]),
						int (pAdapter->Address[3]),
						int (pAdapter->Address[4]),
						int (pAdapter->Address[5]));
					return std::move(std::basic_string<CharT>(mac));
				}
			}

			return std::basic_string<CharT>();
		}

		template < typename CharT >
		std::basic_string<CharT> get_mac_addr(const CharT *ip)
		{
			std::basic_string<CharT> val(ip);
			return get_mac_addr(val);
		}

		/**
		* @brief ���ݱ���DNS
		* @param <dns> <��Ҫ���dns����>
		* @exception <���κ��쳣�׳�>
		* @return <����ɹ��򷵻�true�����򷵻�false>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		inline bool get_local_dns(std::vector<std::basic_string<CharT>> &dns)
		{
			FIXED_INFO fixed = {0};
			ULONG outBufLen = sizeof(FIXED_INFO);

			std::vector<char> tmpBuf;
			FIXED_INFO *tmpFixedInfo = 0;
			if( ::GetNetworkParams(&fixed, &outBufLen) == ERROR_BUFFER_OVERFLOW ) 
			{
				tmpBuf.resize(outBufLen);
				tmpFixedInfo = reinterpret_cast<FIXED_INFO *>(tmpBuf.data());
			}
			else
			{
				tmpFixedInfo = &fixed;
			}

			IP_ADDR_STRING *pIPAddr = 0;
			DWORD dwRetVal = 0;
			if( dwRetVal = ::GetNetworkParams(tmpFixedInfo, &outBufLen) == NO_ERROR ) 
			{
				std::basic_string<CharT> &&tmp = unicode::to(std::basic_string<CharT>(tmpFixedInfo->DnsServerList.IpAddress.String));
				dns.push_back(tmp);

				pIPAddr = tmpFixedInfo->DnsServerList.Next;
				while (pIPAddr) 
				{
					pIPAddr = tmpFixedInfo->DnsServerList.Next;
					tmp = std::move(unicode::to(std::basic_string<CharT>(pIPAddr->IpAddress.String)));
					dns.push_back(std::move(tmp));

					pIPAddr = pIPAddr->Next;
				}
			}


			return true;
		}


		/**
		* @brief MAC�ַ���ת�����Ƶ�ַ
		* @param <mac> <MAC��ַ�ַ���>
		* @exception <���κ��쳣�׳�>
		* @return <���ض�����>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		std::basic_string<CharT> mac_string_to_binary(const std::basic_string<CharT> &mac)
		{
			if( mac.empty() )
				return std::basic_string<CharT>();

			static const size_t len = ::strlen("00-12-EF-AC-0A-78");

			if( mac.length() != len ) 
				return std::basic_string<CharT>();

			std::basic_string<CharT> tmp;
			for(size_t i = 0; i < 6; ++i) 
			{
				tmp += (mac[i*3] - (mac[i*3] >= 'A' ? ('A'-10) : '0')) * 16;
				tmp += mac[i*3+1] - (mac[i*3+1] >= 'A' ? ('A'-10) : '0');
			}

			return tmp;
		}


		/**
		* @brief ����IP�ж��Ƿ�Ϊ����IP
		* @param <ip> <ip��ַ>
		* @exception <���κ��쳣�׳�>
		* @return <����Ǳ����򷵻�true�����򷵻�false>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		inline bool is_local_by_ip(const std::basic_string<CharT> &ip)
		{
			if( ip == utility::select<CharT>("0.0.0.0", L"0.0.0.0") )
				return false;

			if( ip == utility::select<CharT>("127.0.0.1", L"127.0.0.1") )
				return true;

			std::vector<std::basic_string<CharT>> IPs;
			if( !get_local_ips(IPs) )
				return false;

			return std::find(IPs.begin(), IPs.end(), ip) != IPs.end();
		}

		template < typename CharT >
		inline bool is_local_by_ip(const CharT *ip)
		{
			std::basic_string<CharT> val(ip);
			return is_local_by_ip(val);
		}

		/**
		* @brief ���ݻ������ж��Ƿ�Ϊ����IP
		* @param <name> <��������>
		* @exception <���κ��쳣�׳�>
		* @return <����Ǳ����򷵻�true�����򷵻�false>
		* @note <��>
		* @remarks <��>
		*/
		template < typename CharT >
		inline bool is_local_by_machine_name(const std::basic_string<CharT> &name)
		{
			hostent *remoteHost = ::gethostbyname(unicode::to_a(name).c_str());
			if( remoteHost == 0 )
				return false;

			in_addr addr = {0};
			addr.s_addr = *(u_long *)remoteHost->h_addr_list[0];

			std::string ip = ::inet_ntoa(addr);
			return is_local_by_ip(ip);
		}

		template < typename CharT >
		inline bool is_local_by_machine_name(const CharT *name)
		{
			std::basic_string<CharT> val(name);
			return is_local_by_machine_name(val);
		}
		
		/**
		* @brief ����SOCKET���ظ�SOCKET��IP
		* @param <hSocket> <socket���>
		* @exception <���κ��쳣�׳�>
		* @return <����socket�ľ��>
		* @note <��>
		* @remarks <��>
		*/
		inline std::uint32_t get_sck_ip(SOCKET hSocket)
		{
			assert(hSocket != NULL && hSocket != INVALID_SOCKET);
			sockaddr_in addr = {0};
			int len = sizeof(addr);
			if (::getpeername(hSocket, reinterpret_cast<PSOCKADDR>(&addr), &len) != 0)
			{
				//assert(0 && "������׽���");
			}
			return addr.sin_addr.S_un.S_addr;
		}

		/**
		* @brief �ж�ָ���˿��Ǳ�ռ��
		* @param <port> <ָ���Ķ˿ں�>
		* @exception <���κ��쳣�׳�>
		* @return <�����ռ����Ϊtrue������Ϊfalse>
		* @note <����˿ں�Ϊ������>
		* @remarks <��>
		*/
		inline bool is_port_used(std::uint16_t port)
		{
			std::vector<MIB_TCPTABLE> tcp_table;
			tcp_table.resize(100);

			DWORD table_size = sizeof(MIB_TCPTABLE) * (DWORD)tcp_table.size();
			DWORD ret = ::GetTcpTable(&tcp_table[0], &table_size, TRUE);

			if( ret == ERROR_INSUFFICIENT_BUFFER )
			{
				tcp_table.resize(table_size / sizeof(MIB_TCPTABLE));
				ret = ::GetTcpTable(&tcp_table[0], &table_size, TRUE);
				assert(ret == NO_ERROR);
			}
			else if( ret == NO_ERROR )
			{
				size_t mib_cnt = tcp_table[0].dwNumEntries;
				for(size_t i = 0; i != mib_cnt; ++i)
				{
					MIB_TCPROW tcp_row = tcp_table[0].table[i];
					std::uint16_t tmp_port = ::ntohs((std::uint16_t)tcp_row.dwLocalPort);
					if( tmp_port == port )
						return true;
				}
			}
			

			return false;
		}

	}
}




#endif