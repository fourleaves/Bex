#ifndef __BEX_CONFIG_STL_HPP__
#define __BEX_CONFIG_STL_HPP__

//////////////////////////////////////////////////////////////////////////
/// ���Ͽ�������ĸ��汾STL�Ĳ���
#include <iosfwd>

namespace std
{

#if !defined(_MSC_VER)
    const streamoff _BADOFF = -1;
#endif

}

#endif //__BEX_CONFIG_STL_HPP__