#include "TestPCH.h"
#include <Bex/type_traits/class_info.hpp>
using namespace Bex;

BEX_TT_HAS_CONSTEXPR(has_const_int_sci, sci);
BEX_TT_HAS_CONSTEXPR(has_const_int_enm, enm);

template <int >
struct A;

template <>
struct A<1>
{
    static const int sci = 0;
};

BOOST_AUTO_TEST_SUITE(s_class_info)

/// ��ȷ�Բ���
BOOST_AUTO_TEST_CASE(t_class_info)
{
    XDump("��ʼ���� class_info");

    DumpX(has_const_int_sci<A<1> >::value);

    XDump("�������� class_info");
}

BOOST_AUTO_TEST_SUITE_END()