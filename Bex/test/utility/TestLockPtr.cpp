#include "TestPCH.h"
#include <Bex/utility/lock_ptr.hpp>
#include <boost/thread.hpp>
using namespace Bex;

struct A
{
    void func() { Dump(__FUNCTION__); }
    void func() const { Dump("const " << __FUNCTION__); }
    void vf() {}
};

lock_ptr<A> getLockPtr(A & obj, boost::mutex & mu)
{
    return lock_ptr<A>(&obj, mu);
}

BOOST_AUTO_TEST_SUITE(s_lock_ptr)

/// ��ȷ�Բ���
BOOST_AUTO_TEST_CASE(t_lock_ptr)
{
    XDump("��ʼ���� lock_ptr");

    boost::mutex mu;
    A obj;
    {
        lock_ptr<A> lp(&obj, mu);
        BOOST_CHECK(!mu.try_lock());
        lp->func();
    }
    BOOST_CHECK(mu.try_lock());
    mu.unlock();

    {
        lock_ptr<const A> lp(&obj, mu);
        BOOST_CHECK(!mu.try_lock());
        lp->func();
        //lp->vf();
    }

    {
        lock_ptr<A> lp = getLockPtr(obj, mu);
        BOOST_CHECK(!mu.try_lock());
        lp->func();
    }
    BOOST_CHECK(mu.try_lock());
    mu.unlock();

    XDump("�������� lock_ptr");
}

BOOST_AUTO_TEST_SUITE_END()