#include "thread_pool.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/timer.hpp>
#include <Bex/config.hpp>
#if defined(BEX_SUPPORT_CXX11)
# include <atomic>
#endif

namespace Bex
{
    class ThreadPool::thread_pool_impl
    {
    public:
        typedef  ThreadPool::Task  Task;

        class TSQueue
        {
        public:
            inline void push(Task const& task)
            {
                boost::mutex::scoped_lock lock(m_mutex);
                m_taskList.push_back(task);
            }

            inline void pop(Task & out_task)
            {
                if ( empty() )
                {
                    out_task = 0;
                    return ;
                }

                boost::mutex::scoped_lock lock(m_mutex);
                if ( empty() )
                {
                    out_task = 0;
                    return ;
                }

                out_task = m_taskList.front();
                m_taskList.pop_front();
            }

            inline bool empty() const
            {
                return m_taskList.empty();
            }

        private:
            std::list<Task>         m_taskList;     ///< �������
            boost::mutex            m_mutex;        ///< ���������
        };

    public:
        thread_pool_impl(uint32_t threads)
            : m_unfinished(0)
        {
            if (!threads)
                threads = boost::thread::hardware_concurrency();

            for ( uint32_t ui = 0; ui < threads; ++ui )
                m_tg.create_thread(boost::bind(&thread_pool_impl::run, this));
        }

        ~thread_pool_impl()
        {
            m_tg.interrupt_all();
            m_tg.join_all();
        }

        /// ��������̳߳���
        inline void push_task(Task const& task)
        {
#if !defined(BEX_SUPPORT_CXX11)
            BOOST_INTERLOCKED_INCREMENT(&m_unfinished);
#else
            ++ m_unfinished;
#endif
            m_taskQueue.push(task);
        }

        /// �ȴ��������������
        inline bool join_all(uint64_t milliseconds)
        {
            boost::timer bt;
            while ( unfinished() > 0 )
            {
                if ((std::numeric_limits<uint64_t>::max)() != milliseconds
                    && bt.elapsed() * CLOCKS_PER_SEC > milliseconds)
                    return false;

                sys_sleep(1);
            }

            return true;
        }

        /// ��ǰ�̳߳��Ƿ����
        inline bool is_free() const
        {
            return (0 == m_unfinished);
        }

        /// ��ǰδ��ɵ���������
        inline uint32_t  unfinished() const
        {
            return m_unfinished;
        }

        /// �߳�����
        uint32_t  thread_count() const
        {
            return (uint32_t)m_tg.size();
        }

    private:
        /// ��������(�߳�ִ�к���)
        void run();

    private:
        boost::thread_group     m_tg;           ///< �߳���
        TSQueue                 m_taskQueue;    ///< �̰߳�ȫ���������
#if !defined(BEX_SUPPORT_CXX11)
        volatile long           m_unfinished;   ///< δ�����������
#else
        std::atomic<long>       m_unfinished;
#endif
    };

    void ThreadPool::thread_pool_impl::run()
    {
        while ( true )
        {
            if ( m_taskQueue.empty() )
            {
                sys_sleep(1);
                continue;
            }

            Task task;
            m_taskQueue.pop(task);
            if ( !task )
                continue;

            task();
#if !defined(BEX_SUPPORT_CXX11)
            BOOST_INTERLOCKED_DECREMENT(&m_unfinished);
#else
            --m_unfinished;
#endif
        }
    }


    ThreadPool::ThreadPool( uint32_t threads /*= 0*/ )
        : m_Impl( new thread_pool_impl(threads) )
    {

    }

    ThreadPool::~ThreadPool()
    {
        if ( m_Impl ) delete m_Impl, m_Impl = NULL;
    }

    void ThreadPool::push_task( Task const& task )
    {
        m_Impl->push_task(task);
    }

    bool ThreadPool::join_all(uint64_t milliseconds)
    {
        return m_Impl->join_all(milliseconds);
    }

    bool ThreadPool::is_free() const
    {
        return m_Impl->is_free();
    }

    uint32_t ThreadPool::unfinished() const
    {
        return m_Impl->unfinished();
    }

    uint32_t ThreadPool::thread_count() const
    {
        return m_Impl->thread_count();
    }

}