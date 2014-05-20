#include "TestPCH.h"
#include <Bex/platform.hpp>
using namespace Bex;

enum
{
    EC_Big5 = EC_Extend_Begin,
};

namespace Bex
{
    template <>
    struct charset_traits<EC_Big5>
    {
        static const int code_page = 950;
        inline static const char * name() { return "big-5"; }
    };
}

BOOST_AUTO_TEST_SUITE(s_charset_cvt)

/// ��ȷ�Բ���
BOOST_AUTO_TEST_CASE(t_charset_cvt)
{
    XDump("��ʼ���� charset_cvt");

    std::string str = "�Ժ�1234a";

    std::string u8 = a2u8(str);
    std::string u8_check = u82a(u8);
    BOOST_CHECK(str == u8_check);

    std::string big5 = charset_cvt<EC_Local, EC_Big5>(str);    
    std::string big5_check = charset_cvt<EC_Big5, EC_Local>(big5);
    BOOST_CHECK(str == big5_check);

    std::wstring u16 = a2w(str);
    std::string u16_check = w2a(u16);
    BOOST_CHECK(str == u16_check);

    std::wstring tmp = u82w(u8);
    u8_check = w2u8(tmp);
    BOOST_CHECK(u8 == u8_check);

    XDump("�������� charset_cvt");
}

BOOST_AUTO_TEST_SUITE_END()