#ifndef __BEX_NETWORK_COBWEBS_TCP_ACCEPTOR_H__
#define __BEX_NETWORK_COBWEBS_TCP_ACCEPTOR_H__

//////////////////////////////////////////////////////////////////////////
/// tcp���ӽ�����

#include <Bex/network/cobwebs/core/core.h>
#include <Bex/network/cobwebs/session/tcp_session.h>
#include <Bex/network/cobwebs/core/options.h>
#include <Bex/network/cobwebs/core/register.hpp>
#include <Bex/utility/lock_ptr.hpp>
#include "acceptor_base.h"
#include <map>

namespace Bex { namespace cobwebs
{
    class tcp_acceptor
        : public acceptor_base
    {
        typedef boost::shared_ptr<tcp::socket> socket_ptr;
        typedef boost::function<tcp_session_ptr(socket_ptr, session_initialized)> FactoryFunc;
        typedef std::map<session_id, tcp_session_ptr> Sessions;

        group_id                m_gid;
        options_ptr             m_opts;           ///< ����ѡ��
        io_service &            m_ios;
        tcp::acceptor           m_acceptor;       ///< 
        tcp::endpoint           m_endpoint;       ///< 
        FactoryFunc             m_factory;        ///< session��������
        inter_lock              m_lock;           ///< ��ֹ�����������
        Sessions                m_sessions;       ///< session����
        boost::recursive_mutex  m_session_mutex;  ///< session�����߳���
        CRegisterCore           m_register_core;  // -- ����ʱҪ���һ����ʼ��, ����ʱ��һ������.

    public:
        explicit tcp_acceptor(options const& opts = options());
        virtual ~tcp_acceptor();

        /// ��������ѡ��
        void set_option(options const& opts);

        /// ����
        template <class Session>
        bool startup(int port, std::string const& hostname = "0.0.0.0", bool reuse = false)
        {
            return startup(port, session_factory<Session>(), hostname, reuse);
        }

        /// ����
        template <class Factory>
        bool startup(int port, Factory fact, std::string const& hostname = "0.0.0.0", bool reuse = false)
        {
            m_factory = boost::bind(&invoke_factory<tcp_session, Factory>, fact, _1, _2);
            return startup(port, hostname, reuse);
        }

        /// ���ŵعر�acceptor, ͬʱ�ر�����ͨ����acceptor������session.
        void shutdown();

        /// ǿ�ƹر�acceptor
        void close();

        /// ��ȡsessions
        lock_ptr<const Sessions> get_sessions();

    private:
        /// ����
        bool startup(int port, std::string const& hostname, bool reuse);

        /// ��ʼ
        void accept();

        /// ���ӳɹ��ص�
        void on_accept(boost::system::error_code const& ec, socket_ptr pSock);

    private:
        /// �����session
        void insert_session(socket_ptr pSock);

        /// ɾ��session
        void erase_session(session_id sid);

        /// ����ʧЧ����
        void run();
    };

} //namespace cobwebs
} //namespace Bex

#endif //__BEX_NETWORK_COBWEBS_TCP_ACCEPTOR_H__