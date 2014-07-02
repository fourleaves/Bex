#ifndef __BEX_IO_CLIENT_HPP__
#define __BEX_IO_CLIENT_HPP__

//////////////////////////////////////////////////////////////////////////
/// ������Э��Ŀͻ���
#include "bexio_fwd.hpp"

namespace Bex { namespace bexio
{
    template <class Session>
    class client
        : boost::noncopyable
    {
    public:
        typedef client<Session> this_type;

        typedef Session session_type;
        typedef shared_ptr<session_type> session_ptr;
        typedef typename session_type::protocol_type protocol_type;
        typedef typename session_type::callback_type callback_type;

        typedef typename protocol_type::socket socket;
        typedef typename protocol_type::socket_ptr socket_ptr;
        typedef typename protocol_type::resolver resolver;
        typedef typename protocol_type::endpoint endpoint;
        typedef typename protocol_type::allocator allocator;
        typedef typename allocator::template rebind<session_type>::other alloc_session_t;
        typedef typename allocator::template rebind<options>::other alloc_options_t;
        typedef typename allocator::template rebind<callback_type>::other alloc_callback_t;

    public:
        client(io_service & ios, options const& opts)
            : ios_(ios), session_(0)
        {
            opts_ = make_shared_ptr<options, alloc_options_t>(opts);
            callback_ = make_shared_ptr<callback_type, alloc_callback_t>();
        }

        ~client()
        {
            terminate();
            while (session_)
                boost::this_thread::sleep( boost::posix_time::milliseconds(1) );
        }

        // ����ʽ����
        bool connect(endpoint const& addr)
        {
            if (is_running())
                return false;

            socket_ptr sp = protocol_type::alloc_socket(ios_);
            sp->connect(addr, ec_);
            if (ec_) 
                return false;

            session_ = make_shared_ptr<session_type, alloc_session_t>();
            session_->initialize(sp, opts_, callback_);
            running_.set();
            return true;
        }

        // �߼��߳�loop�ӿ�
        void run()
        {
            if (session_)
                session_->run();

            if (session_ && session_->is_disconnected())
                session_.reset();
        }

        // �����Ƿ�OK
        bool is_running() const
        {
            return (session_ && !session_->is_disconnected());
        }

        // ��������
        bool send(char const* buffer, std::size_t size)
        {
            if (session_)
                return session_->send(buffer, size);

            return false;
        }

        // ���ŵعر�����
        void shutdown()
        {
            if (session_)
                session_->shutdown();
        }

        // ǿ�Ƶعر�����
        void terminate()
        {
            if (session_)
                session_->terminate();
        }

        // ����ԭ��
        error_code get_error_code() const
        {
            return ec_;
        }

    private:
        io_service & ios_;

        // ����
        session_ptr session_;

        // ���ӳɹ�
        sentry<inter_lock> running_;

        // ������(���Ӵ���)
        error_code ec_;

        // ѡ��
        shared_ptr<options> opts_;

        // �ص�
        shared_ptr<callback_type> callback_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_CLIENT_HPP__