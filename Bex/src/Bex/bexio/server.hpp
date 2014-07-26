#ifndef __BEX_IO_SERVER_HPP__
#define __BEX_IO_SERVER_HPP__

//////////////////////////////////////////////////////////////////////////
/// ������Э��ķ����
#include "bexio_fwd.hpp"
#include "session_list_mgr.hpp"
#include "handlers.hpp"
#include "core.hpp"

namespace Bex { namespace bexio 
{
    template <class Session, class SessionMgr = session_list_mgr<Session> >
    class basic_server
        : boost::noncopyable
    {
    public:
        typedef basic_server<Session> this_type;

        typedef Session session_type;
        typedef SessionMgr session_mgr_type;

        typedef shared_ptr<session_type> session_ptr;
        typedef typename session_type::id session_id;
        typedef typename session_type::protocol_type protocol_type;
        typedef typename session_type::callback_type callback_type;
        typedef typename session_type::mstrand_service_type mstrand_service_type;

        typedef typename protocol_type::socket socket;
        typedef typename protocol_type::socket_ptr socket_ptr;
        typedef typename protocol_type::acceptor acceptor;
        typedef typename protocol_type::endpoint endpoint;
        typedef typename protocol_type::allocator allocator;
        typedef typename protocol_traits<protocol_type> protocol_traits_type;

        typedef boost::function<void(error_code const&, endpoint const&)> OnHandshakeError;

    public:
        explicit basic_server(options const& opts
            , io_service & ios = core<allocator>::getInstance().backend())
            : ios_(ios), acceptor_(ios)
        {
            opts_ = make_shared_ptr<options, allocator>(opts);
            callback_ = make_shared_ptr<callback_type, allocator>();
            live_cond_ = make_shared_ptr<origin_condition_type, allocator>(true);
        }

        ~basic_server()
        {
            live_cond_->reset();
            terminate();
        }

        // ����
        bool startup(endpoint const& addr, std::size_t listen_count = 1)
        {
            if (is_running())
                return false;

            acceptor_.open(addr.protocol(), ec_);
            if (ec_) return false;

            acceptor_.set_option(socket_base::reuse_address(false), ec_);
            if (ec_) return false;

            acceptor_.bind(addr, ec_);
            if (ec_) return false;

            acceptor_.listen(socket_base::max_connections, ec_);
            if (ec_) return false;

            accept_count_ = listen_count;
            for (std::size_t i = 0; i < listen_count; ++i)
                if (!async_accept())
                    return false;

            // ���������߳�
            use_service<mstrand_service_type>(ios_).startup(opts_->workthread_count);

            running_.set();
            return true;
        }

        // ���ŵعر�
        void shutdown()
        {
            // ����shutdown���
            if (!shutdowning_.set())
                return ;

            // ��ֹͣ��������
            stop_accept();
        }

        // ǿ�ƹر�
        void terminate()
        {
            stop_accept();
            terminate_sessions();
        }

        // �Ƿ�����
        bool is_running() const
        {
            return running_.is_set();
        }

        // ����id��������
        session_ptr find(session_id id) const
        {
            return session_mgr_.find(id);
        }

        // ��ǰ������
        std::size_t size() const
        {
            return session_mgr_.size();
        }

        // ��ȡ������Ϣ
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
        // ���������������
        bool async_accept(bool reply = false)
        {
            if (!reply)
            {
                BOOST_INTERLOCKED_INCREMENT(&accept_count_);
            }

            error_code ec;
            socket_ptr sp = protocol_type::alloc_socket(ios_, *opts_, ec);
            if (ec || !sp)
            {
                if (ec && !ec_)
                    ec_ = ec;
                return false;
            }

            acceptor_.async_accept(sp->lowest_layer(), 
                condition_handler(live_cond_, BEX_IO_BIND(&this_type::on_async_accept, this, BEX_IO_PH_ERROR, sp)));
            return true;
        }

        // ������������ص�
        void on_async_accept(error_code const& ec, socket_ptr sp)
        {
            if (ec)
            {
                if (BOOST_INTERLOCKED_DECREMENT(&accept_count_) == 1 && shutdowning_.is_set())
                    shutdown_sessions();
                else
                    async_accept(true);

                return ;
            }

            async_accept(true);
            async_handshake(sp);
        }

        // ����
        void async_handshake(socket_ptr const& sp)
        {
            BOOST_AUTO(handler, BEX_IO_BIND(&this_type::on_async_handshake, this, BEX_IO_PH_ERROR, sp));
            if (opts_->ssl_opts)
            {
                BOOST_AUTO(timed_handler, timer_handler<allocator>(handler, ios_));
                timed_handler.expires_from_now(boost::posix_time::milliseconds(opts_->ssl_opts->handshake_overtime));
                timed_handler.async_wait(BEX_IO_BIND(&this_type::on_async_handshake, this, generate_error(bee::handshake_overtime), sp));
                protocol_traits_type::async_handshake(sp, ssl::stream_base::server, timed_handler);
            }
            else
                protocol_traits_type::async_handshake(sp, ssl::stream_base::server, handler);
        }

        // ���ֻص�
        void on_async_handshake(error_code const& ec, socket_ptr sp)
        {
            if (ec)
            {
                if (on_handshake_error_)
                {
                    use_service<mstrand_service_type>(ios_).actor().post(BEX_IO_BIND(
                        on_handshake_error_, ec, sp->lowest_layer().remote_endpoint()));
                }
                return ;
            }

            /// create session
            session_type * session_p = allocate<session_type, allocator>();
            session_ptr session(session_p
                , if_else_handler(live_cond_, BEX_IO_BIND(&this_type::session_deleter, this, _1), deallocator<session_type, allocator>())
                , allocator());
            session_p->initialize(sp, opts_, callback_);
            session_mgr_.insert(session);
        }

        // ֹͣ��������
        void stop_accept()
        {
            error_code ec;
            acceptor_.cancel(ec);
            acceptor_.close(ec);
        }

        // ����ɾ����
        void session_deleter(session_type * sp)
        {
            session_mgr_.erase(sp);
            if (!session_mgr_.size())
            {
                running_.reset();
                shutdowning_.reset();
            }

            deallocate<allocator>(sp);
        }

        // ���ŵعر���������
        void shutdown_sessions()
        {
            session_mgr_.for_each(BEX_IO_BIND(&session_type::shutdown, _1));
        }

        // ǿ�Ƶعر���������
        void terminate_sessions()
        {
            session_mgr_.for_each(BEX_IO_BIND(&session_type::terminate, _1));
        }

    private:
        io_service & ios_;

        // acceptor
        acceptor acceptor_;

        // ���ӹ�����
        session_mgr_type session_mgr_;

        // �Ƿ�����
        sentry<inter_lock> running_;

        // ������(ֻ��¼��һ������)
        error_code ec_;

        // ִ���е�accept������
        volatile long accept_count_;

        // ���ŵعر���
        sentry<bool> shutdowning_;

        // ѡ��
        shared_ptr<options> opts_;

        // �ص�
        shared_ptr<callback_type> callback_;

        // ���ֳ���ص�
        OnHandshakeError on_handshake_error_;

        // loopģʽ�Ļص�����
        io_service callback_list;

        // ��ʶ�����Ƿ����condition
        handler_condition_type live_cond_;
    };
    

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SERVER_HPP__