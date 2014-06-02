#ifndef __BEX_SIGNALS_SIGNAL__
#define __BEX_SIGNALS_SIGNAL__

#include "signal_fwd.h"
#include "combine_last_value.hpp"
#include "combine_accumulate_value.hpp"

//////////////////////////////////////////////////////////////////////////
/// ����signals2�Ĺ۲���ģʽ���

namespace Bex
{
    template <typename Signature>
    struct sign_wrapper
    {
        typedef Signature type;
    };

    template <int Group, int Index>
    struct signal_traits;

    template <int Group, int Index>
    struct SignalHolder
    {
        typedef signal_traits<Group, Index> traits;
        typedef boost_signals2::signal<
            typename traits::Signature,
            typename traits::Combiner> signal_type;

        static signal_type & get_signal()
        {
            static signal_type obj;
            return obj;
        }
    };

    namespace
    {
        /// ȡ���һ������ֵ�ĺϲ���
        using detail::combine_last_value;

        /// �ۼ����з���ֵ�ĺϲ���
        using detail::combine_accumulate_value;
    }
}

/// ��ȡָ��(��, ���)��signal.
// @Group ������, �����Ǳ���������, �� const int, enum��.
// @Index ���еı������, �����Ǳ���������, �� const int, enum��.
#define BEX_GET_SIGNAL(Group, Index) (::Bex::SignalHolder<Group, Index>::get_signal())

/// ����signal
// @Signature �ص���������, ��: void(), int(double, void) ��.
#define BEX_DEFINE_SIGNAL(Group, Index, signature)                              \
namespace Bex {                                                                 \
    template <>                                                                 \
    struct signal_traits<Group, Index>                                          \
    {                                                                           \
        typedef sign_wrapper<signature>::type Signature;                        \
        typedef combine_last_value<typename                                     \
             boost::function_traits<signature>::result_type> Combiner;          \
    };                                                                          \
}

/// ����ʹ��ָ���ϲ�����signal
#define BEX_DEFINE_COMBINE_SIGNAL(Group, Index, signature, combiner)            \
namespace Bex {                                                                 \
    template <>                                                                 \
    struct signal_traits<Group, Index>                                          \
    {                                                                           \
        typedef sign_wrapper<signature>::type Signature;                        \
        typedef combiner<typename                                               \
            boost::function_traits<signature>::result_type> Combiner;           \
    };                                                                          \
}


#endif //__BEX_SIGNALS_SIGNAL__