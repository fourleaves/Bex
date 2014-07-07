#ifndef __BEX_IO_SESSION_LIST_MGR_HPP__
#define __BEX_IO_SESSION_LIST_MGR_HPP__

//////////////////////////////////////////////////////////////////////////
/// ���ӹ�����(����ʽ˫�������洢)
/*
* @ ��У��session����, ʹ����Ҫ����!
*/
#include "bexio_fwd.hpp"
#include "intrusive_list.hpp"

namespace Bex { namespace bexio
{
    template <class Session, typename LockType = inter_lock>
    class session_list_mgr
    {
    public:
        typedef session_list_mgr<Session, LockType> this_type;
        typedef Session session_type;
        typedef typename session_type::id session_id;
        typedef shared_ptr<session_type> session_ptr;
        typedef intrusive_list<session_type> list_type;
        typedef LockType lock_type;

        session_list_mgr() : size_(0) {}
        ~session_list_mgr()
        {
            list_.clear();
            size_ = 0;
        }

        /// ��������id
        static session_id create_id(session_ptr sp)
        {
            return session_id(sp);
        }

        /// ����
        void insert(session_ptr sp)
        {
            if (!sp) return ;

            BOOST_INTERLOCKED_INCREMENT(&size_);
            typename lock_type::scoped_lock lock(lock_);
            list_.push_back(sp.get());
        }

        /// ɾ��
        void erase(session_id id)
        {
            session_ptr sp = id.get();
            if (!sp) return ;

            erase(sp.get());
        }

        /// ɾ��
        void erase(session_type * sp)
        {
            BOOST_INTERLOCKED_DECREMENT(&size_);
            typename lock_type::scoped_lock lock(lock_);
            list_.erase(sp);
        }

        /// ����
        session_ptr find(session_id id) const
        {
            return id.get();
        }

        /// ����
        std::size_t size() const
        {
            return size_;
        }

        template <typename F>
        void for_each(F f)
        {
            typename lock_type::scoped_lock lock(lock_);
            typename list_type::iterator it = list_.begin();
            for (; it != list_.end(); ++it)
                f(*it);
        }

    private:
        list_type list_;
        volatile long size_;
        lock_type lock_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SESSION_LIST_MGR_HPP__