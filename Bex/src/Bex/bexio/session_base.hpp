#ifndef __BEX_IO_SESSION_BASE_HPP__
#define __BEX_IO_SESSION_BASE_HPP__

//////////////////////////////////////////////////////////////////////////
/// ���������
#include "bexio_fwd.hpp"

namespace Bex { namespace bexio
{
    template <class SessionMgr>
    class session_base
        : public SessionMgr::hook
        , public boost::enable_shared_from_this<session_base<SessionMgr> >
        , boost::noncopyable
    {
        friend SessionMgr;

    public:
        typedef SessionMgr session_mgr_type;
        typedef typename SessionMgr::session_id id;
        
        virtual ~session_base() {}

        /// ��ʼ��(Ͷ�ݽ�������)
        virtual void initialize() = 0;

        /// ���ŵعر�����
        virtual void shutdown() = 0;

        /// ǿ�ƹر�����
        virtual void terminate() = 0;

        // ��ȡid
        id get_id() const
        {
            return session_mgr_type::create_id(shared_from_this());
        }

    private:
        /// session id
        long id_;
        static volatile long svlid;
    };

    template <class SessionMgr>
    volatile long session_base<SessionMgr>::svlid = 1;

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SESSION_BASE_HPP__