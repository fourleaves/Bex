#ifndef __BEX_THREAD_POOL_HPP__
#define __BEX_THREAD_POOL_HPP__

#include <Bex/platform.hpp>
#include <Bex/base.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

//////////////////////////////////////////////////////////////////////////
/// �̳߳�

namespace Bex
{
    class ThreadPool
        : public boost::noncopyable
    {
    public:
        typedef  boost::function<void()>  Task;

        ThreadPool(uint32_t threads = 0);
        ~ThreadPool();

        /// ��������̳߳���
        void push_task(Task const& task);

        /// �ȴ��������������
        bool join_all(uint64_t milliseconds = (std::numeric_limits<uint64_t>::max)());

        /// ��ǰ�̳߳��Ƿ����
        bool is_free() const;

        /// ��ǰδ��ɵ���������
        uint32_t  unfinished() const;

        /// �߳�����
        uint32_t  thread_count() const;

    private:
        class thread_pool_impl;
        thread_pool_impl * m_Impl;
    };
}

#endif //__BEX_THREAD_POOL_H__