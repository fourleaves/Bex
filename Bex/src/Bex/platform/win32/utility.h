#ifndef __BEX_PLATFORM_WIN32_UTILITY_H__
#define __BEX_PLATFORM_WIN32_UTILITY_H__

#include <boost/logic/tribool.hpp>

namespace Bex
{
    // ��ǰִ�еĴ����Ƿ���Dll��.
    boost::tribool is_in_dll();

} //namespace Bex

#endif //__BEX_PLATFORM_WIN32_UTILITY_H__