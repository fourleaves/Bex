#include "TestPCH.h"
#include <Bex/utility/format.hpp>
using namespace Bex;

BOOST_AUTO_TEST_SUITE(s_format)

/// ��ȷ�Բ���
BOOST_AUTO_TEST_CASE(t_format)
{
    XDump("��ʼ���� format");

    std::string str = format("%d %s", 2, 4);
    BOOST_CHECK(str == "2 4");

    std::wstring wstr = format(L"%s %s", 2, L"X");
    BOOST_CHECK(wstr == L"2 X");
    
    XDump("�������� format");
}

BOOST_AUTO_TEST_SUITE_END()