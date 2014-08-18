#ifndef __BEX_FAST_ALLOCATOR_HPP__
#define __BEX_FAST_ALLOCATOR_HPP__

#include <boost/pool/poolfwd.hpp>
#include <boost/pool/pool_alloc.hpp>

namespace Bex
{
    // �̰߳�ȫ��allocator
    template <typename T>
    using fast_pool_allocator = ::boost::fast_pool_allocator<T>;

    // ���̰߳�ȫ��allocator, ����,����!!!
    template <typename T>
    using nonblocking_fast_pool_allocator = ::boost::fast_pool_allocator<T, 
        boost::default_user_allocator_new_delete,
        boost::details::pool::null_mutex>;
} //namespace Bex

#endif //__BEX_FAST_ALLOCATOR_HPP__