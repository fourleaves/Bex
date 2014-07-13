#ifndef __BEX_IO_CLIENT_HPP__
#define __BEX_IO_CLIENT_HPP__

//////////////////////////////////////////////////////////////////////////
/// ������Э��Ŀͻ���
#include "bexio_fwd.hpp"

namespace Bex { namespace bexio
{
    template <class Session>
    class basic_client
        : boost::noncopyable
    {
    public:
        typedef basic_client<Session> this_type;

        typedef Session session_type;
        typedef shared_ptr<session_type> session_ptr;
        typedef typename session_type::protocol_type protocol_type;
        typedef typename session_type::callback_type callback_type;
        typedef typename session_type::mstrand_service_type mstrand_service_type;

        typedef typename protocol_type::socket socket;
        typedef typename protocol_type::socket_ptr socket_ptr;
        typedef typename protocol_type::resolver resolver;
        typedef typename protocol_type::endpoint endpoint;
        typedef typename protocol_type::allocator allocator;
        typedef typename protocol_traits<protocol_type> protocol_traits_type;

        typedef boost::function<void(error_code const&)> OnAsyncConnect;
        typedef boost::function<void(error_code const&, endpoint const&)> OnHandshakeError;

    public:
        basic_client(io_service & ios, options const& opts)
            : ios_(ios), session_(0), mstrand_service_(0)
        {
            opts_ = make_shared_ptr<options, allocator>(opts);
            callback_ = make_shared_ptr<callback_type, allocator>();

            if (opts_->nlte_ == nlte::nlt_reactor)
                mstrand_service_ = &use_service<mstrand_service_type>(ios);
        }

        ~basic_client()
        {
            terminate();
            while (session_)
                boost::this_thread::sleep( boost::posix_time::milliseconds(1) );
        }

        // ����ʽ����
        bool connect(endpoint const& addr)
        {
            if (is_running() || async_connecting_.is_set())
                return false;

            socket_ptr sp = protocol_type::alloc_socket(ios_, *opts_, ec_);
            if (ec_)
                return false;

            sp->lowest_layer().connect(addr, ec_);
            if (ec_) 
                return false;

            protocol_traits_type::handshake(sp, ssl::stream_base::client, ec_);
            if (ec_) 
                return false;

            session_ = make_shared_ptr<session_type, allocator>();
            session_->initialize(sp, opts_, callback_);

            // ���������߳�
            use_service<mstrand_service_type>(ios_).startup(opts_->workthread_count);
            running_.set();
            return true;
        }

        // �����첽���ӻص�
        void set_async_connect_callback(OnAsyncConnect const& callback)
        {
            async_connect_callback_ = callback;
        }

        // �첽����
        bool async_connect(endpoint const& addr)
        {
            if (is_running() || !async_connecting_.set())
                return false;

            socket_ptr sp = protocol_type::alloc_socket(ios_, *opts_, ec_);
            if (ec_)
                return false;

            sp->lowest_layer().async_connect(addr, 
                BEX_IO_BIND(&this_type::on_async_connect, this, BEX_IO_PH_ERROR
                    , sp, addr));

            // ���������߳�
            use_service<mstrand_service_type>(ios_).startup(opts_->workthread_count);
            return true;
        }

        // ����ʱ���첽����
        bool async_connect_timed(endpoint const& addr, boost::posix_time::time_duration timed)
        {
            if (is_running() || !async_connecting_.set())
                return false;

            socket_ptr sp = protocol_type::alloc_socket(ios_, *opts_, ec_);
            if (ec_)
                return false;

            /// ��ʱ���
            shared_ptr<sentry<inter_lock> > overtime_token = make_shared_ptr<sentry<inter_lock>, allocator>();

            /// ���ӳ�ʱ��ʱ��, �첽�ȴ�
            shared_ptr<deadline_timer> timer_ = make_shared_ptr<deadline_timer, allocator>(ios_);
            timer_->expires_from_now(timed);
            timer_->async_wait(BEX_IO_BIND(&this_type::on_overtime, this, BEX_IO_PH_ERROR, sp, overtime_token, timer_));

            sp->lowest_layer().async_connect(addr, 
                BEX_IO_BIND(&this_type::on_async_connect, this, BEX_IO_PH_ERROR, sp, addr, overtime_token, timed));

            // ���������߳�
            use_service<mstrand_service_type>(ios_).startup(opts_->workthread_count);
            return true;
        }

        // �߼��߳�loop�ӿ�
        void run()
        {
            if (notify_async_connect_.reset() && async_connect_callback_)
                async_connect_callback_(ec_);

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
            if (is_running())
                return session_->send(buffer, size);

            return false;
        }

        // ���ŵعر�����
        void shutdown()
        {
            if (is_running())
                session_->shutdown();
        }

        // ǿ�Ƶعر�����
        void terminate()
        {
            if (is_running())
                session_->terminate();
        }

        // ����ԭ��
        error_code get_error_code() const
        {
            return ec_;
        }

        // ����������Ϣ�ص�
        template <typename session_type::callback_em CallbackType, typename F>
        void set_callback(F const& f)
        {
            session_type::set_callback(*callback_, f);
        }

        // �������ֳ���ص�
        void set_handshake_error_callbcak(OnHandshakeError const& f)
        {
            on_handshake_error_ = f;
        }

    private:
        // �첽���ӻص�
        void on_async_connect(error_code const& ec, socket_ptr sp, endpoint addr)
        {
            if (ec)
            {
                // ����
                ec_ = ec;
                async_connecting_.reset();
                notify_onconnect();    // ֪ͨ���ӽ��
            }
            else if (sp->lowest_layer().remote_endpoint() == sp->lowest_layer().local_endpoint())
            {
                // �ػ�������, ����.
                async_connecting_.reset();
                if (!async_connect(addr))
                {
                    if (!ec_) ec_ = generate_error(bee::reconnect_error);
                    notify_onconnect();
                }
            }
            else
            {
                // ���ӳɹ�, ����
                async_handshaking_.set();
                protocol_traits_type::async_handshake(sp, ssl::stream_base::client
                    , BEX_IO_BIND(&this_type::on_async_handshake, this, BEX_IO_PH_ERROR, sp, addr));
            }
        }

        // ���ֻص�
        void on_async_handshake(error_code const& ec, socket_ptr sp, endpoint addr)
        {
            async_handshaking_.reset();
            if (ec)
            {
                ec_ = ec;
                async_connecting_.reset();
                if (on_handshake_error_)
                    on_handshake_error_(ec, sp->lowest_layer().remote_endpoint());
            }
            else
            {
                ec_.clear();
                running_.set();
                async_connecting_.reset();
                session_ = make_shared_ptr<session_type, allocator>();
                session_->initialize(sp, opts_, callback_);
            }

            notify_onconnect();
        }

        // �첽���ӻص�(����ʱ)
        void on_async_connect_timed(error_code const& ec, socket_ptr sp
            , endpoint addr, shared_ptr<sentry<inter_lock> > overtime_token
            , boost::posix_time::time_duration timed)
        {
            if (overtime_token && overtime_token->is_set())
            {
                // ��ʱ��
                ec_ = generate_error(bee::connect_overtime);
                async_connecting_.reset();
                notify_onconnect();
            }
            else if (ec)
            {
                // ����
                ec_ = ec;
                async_connecting_.reset();
                notify_onconnect();
            }
            else if (sp->lowest_layer().remote_endpoint() == sp->lowest_layer().local_endpoint())
            {
                // �ػ�������, ����.
                async_connecting_.reset();
                if (!async_connect_timed(addr, timed))
                {
                    if (!ec_) ec_ = generate_error(bee::reconnect_error);
                    notify_onconnect();
                }
                return ;
            }
            else
            {
                // ���ӳɹ�
                async_handshaking_.set();
                protocol_traits_type::async_handshake(sp, ssl::stream_base::client
                    , BEX_IO_BIND(&this_type::on_async_handshake_timed, this
                        , BEX_IO_PH_ERROR, sp, addr, overtime_token, timed));
            }
        }

        // ���ֻص�(����ʱ)
        void on_async_handshake_timed(error_code const& ec, socket_ptr sp
            , endpoint addr, shared_ptr<sentry<inter_lock> > overtime_token
            , boost::posix_time::time_duration timed)
        {
            async_handshaking_.reset();
            if (overtime_token && !overtime_token->set())
            {
                // ��ʱ��
                ec_ = generate_error(bee::connect_overtime);
                async_connecting_.reset();
            }
            else if (ec)
            {
                ec_ = ec;
                async_connecting_.reset();
            }
            else
            {
                ec_.clear();
                running_.set();
                async_connecting_.reset();
                session_ = make_shared_ptr<session_type, allocator>();
                session_->initialize(sp, opts_, callback_);
            }

            notify_onconnect();
        }

        // ��ʱ�ص�
        void on_overtime(error_code const& ec, socket_ptr sp, shared_ptr<sentry<inter_lock> > overtime_token, shared_ptr<deadline_timer>)
        {
            // @todo: �����봦��
            if (overtime_token->set())
            {
                // ���ӳ�ʱ��
                error_code lec;
                sp->next_layer().cancel(lec);
                sp->lowest_layer().shutdown(socket_base::shutdown_both, lec);
                sp->next_layer().close(lec);
            }
        }

        // ֪ͨ���ӽ��
        void notify_onconnect()
        {
            if (async_connect_callback_)
            {
                if (opts_->nlte_ == nlte::nlt_reactor)
                    mstrand_service_->post(BEX_IO_BIND(async_connect_callback_, ec_));
                else if (opts_->nlte_ == nlte::nlt_loop)
                    notify_async_connect_.set();
            }
        }

    private:
        io_service & ios_;

        // ����
        session_ptr session_;

        // �첽������
        sentry<inter_lock> async_connecting_;

        // ������
        sentry<inter_lock> async_handshaking_;

        // �첽���ӻص�
        OnAsyncConnect async_connect_callback_;

        // �첽���ӻص�֪ͨ
        sentry<bool> notify_async_connect_;

        // ���ӳɹ�
        sentry<inter_lock> running_;

        // ���Ӵ�����
        error_code ec_;

        // �����̷߳���
        mstrand_service_type * mstrand_service_;

        // ѡ��
        shared_ptr<options> opts_;

        // �ص�
        shared_ptr<callback_type> callback_;

        // ���ֳ���ص�
        OnHandshakeError on_handshake_error_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_CLIENT_HPP__