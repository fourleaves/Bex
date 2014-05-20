#ifndef __BEX_NETWORK_COBWEBS_TCP_SESSION_H__
#define __BEX_NETWORK_COBWEBS_TCP_SESSION_H__

//////////////////////////////////////////////////////////////////////////
/// tcp����

#include <Bex/network/cobwebs/session/session_base.hpp>
#include <Bex/network/cobwebs/session/session_factory.hpp>
#include <Bex/network/cobwebs/core/register.hpp>
#include <Bex/stream.hpp>

namespace Bex { namespace cobwebs
{
    class tcp_session
        : public session_base
        , public boost::enable_shared_from_this<tcp_session>
    {
        template <typename Session>
        friend class session_factory;

    public:
        typedef tcp::socket socket_type;
        typedef boost::shared_ptr<socket_type> socket_ptr;
        
    private:
        socket_ptr m_socket;                // tcp socket
        multi_ringbuf m_sendbuf;            // ���ͻ�����
        multi_ringbuf m_recvbuf;            // ���ջ�����
        boost::mutex m_send_mutex;          // ����������(send�ӿ�һ��ֻ��һ���̵߳���)

        bool       m_notify_connected;      // �Ƿ���Ҫ����on_connected�ص�
        inter_lock m_receive_relay;         // �����������
        inter_lock m_send_relay;            // �����������
        
        inter_lock m_shutdown_lock;         // ���ŵعر���...
        inter_lock m_receive_closed;        // ����ͨ���ѹر�
        inter_lock m_send_closed;           // ����ͨ���ѹر�
        inter_lock m_both_closed;           // socket�ѹر�(close()�л�����)

        inter_lock m_set_error_lock;        // ������Ϣ������(���Ա�ֻ֤������һ��)
        boost::system::error_code m_error;  // ������Ϣ

        CRegisterCore m_register_core;      // -- ����ʱҪ���һ����ʼ��, ����ʱ��һ������.

    public:
        tcp_session(socket_ptr pSock, session_initialized const& si);
        ~tcp_session();

        /// ��������
        virtual bool send(char const* buf, std::size_t bytes);

    public:
        /// �����߳��ƽ�
        virtual void run();

        /// ���ŵعر�����(����)
        virtual void shutdown();

        /// ǿ�ƹر�
        virtual void close();

    protected:
        /// ���ӳɹ��ص�
        virtual void on_connected() = 0;

        /// �������ݳɹ��ص�
        virtual void on_recv(char * buf, std::size_t bytes) = 0;

        /// �Ͽ����ӻص�
        virtual void on_disconnect(boost::system::error_code const& ec) = 0;

    private:
        /// ��ʼ��
        void initialize();

        /// �����������
        void post_recv(bool relay);

        /// ����������
        void post_send(bool relay);

        /// @{ �����ڵ���, ��������д
    protected:
        /// ���ջص�
        virtual void recv_handler(boost::system::error_code const& ec, std::size_t bytes);

        /// ���ͻص�
        virtual void send_handler(boost::system::error_code const& ec, std::size_t bytes);
        /// }@

    private:
        /// ���ӳɹ�
        void connect_invoke();

        /// �Ͽ�����
        void set_disconnect_error(boost::system::error_code const& ec);

        /// ���ŵعر�����(����)
        void passive_shutdown(socket_base::shutdown_type what);

        /// ���Թر�����
        void try_shutdown();
    };

    typedef boost::shared_ptr<tcp_session> tcp_session_ptr;
    typedef boost::weak_ptr<tcp_session> tcp_session_holder;

} //namespace cobwebs
} //namespace Bex

#endif //__BEX_NETWORK_COBWEBS_TCP_SESSION_H__