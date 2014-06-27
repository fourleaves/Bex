#ifndef __BEX_IO_BEXIO_FWD_HPP__
#define __BEX_IO_BEXIO_FWD_HPP__

#include <Bex/config.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/array.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <list>

namespace Bex { namespace bexio
{
    using namespace boost::asio;
    using boost::system::error_code;

    //////////////////////////////////////////////////////////////////////////
    /// @{ meta functions
    /// ���Ͷ���concept���
    template <class T>
    struct has_next_layer
    {
        template <typename U>
        static char _check(...);

        template <typename U>
        static short _check(typename U::next_layer_type*);

        static const bool value = (sizeof(_check<T>(0)) == sizeof(short));
    };

    template <class T>
    struct has_lowest_layer
    {
        template <typename U>
        static char _check(...);

        template <typename U>
        static short _check(typename U::lowest_layer_type*);

        static const bool value = (sizeof(_check<T>(0)) == sizeof(short));
    };

    /// �ֲ������е���һ������
    template <class T>
    typename T::next_layer_type& next_layer(T& object)
    {
        return object.next_layer();
    }

    template <class T>
    typename boost::disable_if<has_next_layer<T>, T&>::type next_layer(T& object, ...)
    {
        return object;
    }

    /// �ֲ������е���ײ�����
    template <class T>
    typename T::lowest_layer_type& lowest_layer(T& object)
    {
        return object.lowest_layer();
    }

    template <class T>
    typename boost::disable_if<has_lowest_layer<T>, T&>::type lowest_layer(T& object, ...)
    {
        return object;
    }

    /// ��ȡ�ֲ������е���һ������
    template <class T, bool>
    struct next_layer_t_helper
        : boost::mpl::identity<typename T::next_layer_type>
    {};

    template <class T>
    struct next_layer_t_helper<T, false>
        : boost::mpl::identity<T>
    {};

    template <class T>
    struct next_layer_t
        : next_layer_t_helper<T, has_next_layer<T>::value >
    {};

    /// ��ȡ�ֲ������е���ײ�����
    template <class T, bool>
    struct lowest_layer_t_helper
        : boost::mpl::identity<typename T::lowest_layer_type>
    {};

    template <class T>
    struct lowest_layer_t_helper<T, false>
        : boost::mpl::identity<T>
    {};

    template <class T>
    struct lowest_layer_t
        : lowest_layer_t_helper<T, has_lowest_layer<T>::value >
    {};

    /// @}
    //////////////////////////////////////////////////////////////////////////

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_BEXIO_FWD_HPP__