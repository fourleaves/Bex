#include "TestPCH.h"
#include <Bex/thread.hpp>

class TestThreadPool
{
public:
    TestThreadPool()
        : m_long(0)
    {
        XDump(" ��ʼ���� ThreadPool" );
    }
    ~TestThreadPool()
    {
        XDump(" �������� ThreadPool" );
    }

    void sleep_m1()
    {
        sys_sleep(1);
    }

    void sleep_1()
    {
        sys_sleep(1000);
        BOOST_INTERLOCKED_INCREMENT(&m_long);
    }

    void sleep_2()
    {
        sys_sleep(2000);
    }

    ThreadPool      m_threadpool;
    volatile long   m_long;
};

BOOST_FIXTURE_TEST_SUITE(s_threadpool, TestThreadPool)

BOOST_AUTO_TEST_CASE(t_threadpool)
{
    BOOST_CHECK( (m_threadpool.thread_count() >= 1) );
    Dump("Thread Count = " << m_threadpool.thread_count())
    
    BOOST_CHECK_EQUAL( m_threadpool.unfinished(), 0 );
    UFOR( m_threadpool.thread_count() )
    {
        m_threadpool.push_task(boost::bind(&TestThreadPool::sleep_1, this));
        BOOST_CHECK_EQUAL( m_threadpool.unfinished(), ui + 1 );
    }

    BOOST_CHECK( !m_threadpool.join_all(0) );
    BOOST_CHECK( !m_threadpool.is_free() );

    sys_sleep(1500);
    BOOST_CHECK_EQUAL( m_threadpool.unfinished(), 0 );
    BOOST_CHECK( m_threadpool.join_all(0) );
    BOOST_CHECK_EQUAL( m_long, m_threadpool.thread_count() );
    BOOST_CHECK( m_threadpool.is_free() );

    m_threadpool.push_task(boost::bind(&TestThreadPool::sleep_1, this));
    BOOST_CHECK_EQUAL( m_threadpool.unfinished(), 1 );
    BOOST_CHECK( m_threadpool.join_all() );
    BOOST_CHECK_EQUAL( m_threadpool.unfinished(), 0 );
    BOOST_CHECK( m_threadpool.is_free() );
}

BOOST_AUTO_TEST_SUITE_END()