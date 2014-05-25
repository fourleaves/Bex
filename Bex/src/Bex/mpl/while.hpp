#ifndef __BEX_MPL_WHILE_HPP__
#define __BEX_MPL_WHILE_HPP__

#include <boost/mpl/if.hpp>
#include <boost/mpl/identity.hpp>
#include <Bex/type_traits/static_debug.hpp>
//////////////////////////////////////////////////////////////////////////
/// simple mpl while

namespace Bex { namespace mpl
{
    //////////////////////////////////////////////////////////////////////////
    /// mpl while
    // @Cond ѭ��������������
    // @Do   ѭ����ִ�к���
    // @T    ѭ������
    // @Return T��ѭ������Ľ��
    template <
        template<typename> class Cond,
        template<typename> class Do,
        typename T
    >
    struct while_
        : boost::mpl::if_<
                Cond<T>, 
                while_<Cond, Do, typename Do<T>::type>, 
                boost::mpl::identity<T>
            >::type
    {
        //BEX_STATIC_DEBUG_TYPEINFO(Cond<T>::value, T);
    };

} // namespace mpl
} // namespace Bex

#endif //__BEX_MPL_WHILE_HPP__