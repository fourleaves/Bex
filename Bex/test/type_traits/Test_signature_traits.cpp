#include "TestPCH.h"
#include <Bex/type_traits/signature_traits.hpp>
using namespace Bex;

class A
{

};

BOOST_AUTO_TEST_SUITE(s_signature_traits)

/// ��ȷ�Բ���
BOOST_AUTO_TEST_CASE(t_signature_traits)
{
    XDump("��ʼ���� signature_traits");

    {
        typedef signature_traits<void()> st;
        DumpX(typeid(st::result_type).name());
        DumpX(typeid(st::function).name());
        DumpX(typeid(st::member_function<A>::type).name());
    }

    {
        typedef void(_cdecl*FT)(void);
        typedef signature_traits<FT> st;
        DumpX(typeid(st::result_type).name());
        DumpX(typeid(st::function).name());
        DumpX(typeid(st::member_function<A>::type).name());
    }

    {
        typedef signature_traits<int(double, float)> st;
        DumpX(typeid(st::result_type).name());
        DumpX(typeid(st::function).name());
        DumpX(typeid(st::member_function<A>::type).name());
    }

    XDump("�������� signature_traits");
}

BOOST_AUTO_TEST_SUITE_END()