#ifndef __MULTI_THREAD_THREAD_POOL_SIZE_CONTROL_HPP
#define __MULTI_THREAD_THREAD_POOL_SIZE_CONTROL_HPP



namespace multi_thread
{
	namespace threadpool
	{

		// ----------------------------------
		// ��̬��С,����������

		template < typename ImplT >
		class static_size
		{
		public:
			void resize(ImplT &/*impl*/, size_t /*cnt*/)
			{
				return;
			}
		};

		// ----------------------------------
		// ��̬����,���ݸ���������

		template < typename ImplT >
		class dynamic_size
		{
		public:
			void resize(ImplT &impl, size_t cnt)
			{
				impl.resize(cnt);
				return;
			}
		};
	}
}






#endif