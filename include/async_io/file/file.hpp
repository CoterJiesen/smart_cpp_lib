#ifndef __FILESYSTEM_FILE_HPP
#define __FILESYSTEM_FILE_HPP

#include "../iocp/async_result.hpp"
#include "../iocp/read_write_buffer.hpp"
#include "../iocp/buffer.hpp"
#include "../iocp/read.hpp"
#include "../iocp/write.hpp"


namespace async
{
	namespace iocp
	{
		class io_dispatcher;
	}


	namespace filesystem
	{
		// forward declare

		class file_handle;
		typedef std::shared_ptr<file_handle> file_handle_ptr;
		

		//--------------------------------------------------------------------------------
		// class File

		class file_handle
		{
		public:
			typedef iocp::io_dispatcher	dispatcher_type;

		private:
			// File Handle
			HANDLE file_;
			// IO����
			dispatcher_type &io_;

		public:
			explicit file_handle(dispatcher_type &);
			file_handle(dispatcher_type &, LPCTSTR, DWORD, DWORD, DWORD, DWORD, LPSECURITY_ATTRIBUTES = NULL, HANDLE = NULL);
			~file_handle();

			// non-copyable
		private:
			file_handle(const file_handle &);
			file_handle &operator=(const file_handle &);


		public:
			// explicitת��
			operator HANDLE()					{ return file_; }
			operator const HANDLE () const		{ return file_; }

			// ��ʾ��ȡ
			HANDLE native_handle()					{ return file_; }
			const HANDLE native_handle() const		{ return file_; }

		public:
			// ��Ŀ���ļ�
			void open(LPCTSTR, DWORD, DWORD, DWORD, DWORD, LPSECURITY_ATTRIBUTES = NULL, HANDLE = NULL);
			// �ر�
			void close();
			
			// �Ƿ��
			bool is_open() const
			{ return file_ != INVALID_HANDLE_VALUE; }

			// ˢ��
			bool flush();
			
			//	ȡ��
			bool cancel();

			// �����ļ���С
			void set_file_size(unsigned long long size);

			// �������ûص��ӿ�,ͬ������
		public:
			size_t read(iocp::mutable_buffer &buffer, const u_int64 &offset);
			size_t write(const iocp::const_buffer &buffer, const u_int64 &offset);

			// �첽���ýӿ�
		public:
			void async_read(iocp::mutable_buffer &buffer, const u_int64 &offset, const iocp::rw_callback_type &handler);
			void async_write(const iocp::const_buffer &buffer, const u_int64 &offset, const iocp::rw_callback_type &handler);
		};
	
		
		file_handle_ptr make_file(file_handle::dispatcher_type &io);
		file_handle_ptr make_file(file_handle::dispatcher_type &io, LPCTSTR path, DWORD access, DWORD shared, DWORD create, DWORD flag, LPSECURITY_ATTRIBUTES attribute = NULL, HANDLE templateMode = NULL);
		
	}
	
}




#endif