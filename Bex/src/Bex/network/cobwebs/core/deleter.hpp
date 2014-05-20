#ifndef __BEX_NETWORK_COBWEBS_CORE_DELETER_HPP__
#define __BEX_NETWORK_COBWEBS_CORE_DELETER_HPP__

//////////////////////////////////////////////////////////////////////////
/// ���ü�����һʱ�������, ת������ָ��ָ��Ķ��������ʱ��.

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <map>
#include "core.h"

namespace Bex { namespace cobwebs
{
    template <typename T>
    class core_deleter;

    /// delete����ת�Ƶ�core����ѭ���д���
    template <typename T>
    struct mainloop_deleter
    {
        inline void operator()(T * ptr) const
        {
            core::getInstance().post(core_deleter<T>(ptr));
        }
    };

    /// *** delete�º���, Ϊ׷��Ч��, ��ȫ�Բ���, ����!!!
    template <typename T>
    class core_deleter
    {
        T * m_pointer;

    public:
        explicit core_deleter(T * pointer)
            : m_pointer(pointer)
        {
        }

        inline void operator()() const
        {
            delete m_pointer;
        }
    };

} //namespace cobwebs
} //namespace Bex

#endif //__BEX_NETWORK_COBWEBS_CORE_DELETER_HPP__