#ifndef __BEX_IO_SERVER_HPP__
#define __BEX_IO_SERVER_HPP__

//////////////////////////////////////////////////////////////////////////
/// ������Э��ķ����
#include "bexio_fwd.hpp"
#include "session_list_mgr.hpp"

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

        typedef typename protocol_type::socket socket;
        typedef typename protocol_type::socket_ptr socket_ptr;
        typedef typename protocol_type::acceptor acceptor;
        typedef typename protocol_type::endpoint endpoint;
        typedef typename protocol_type::allocator allocator;
        typedef typename allocator::template rebind<session_type>::other alloc_session_t;
        typedef typename allocator::template rebind<options>::other alloc_options_t;
        typedef typename allocator::template rebind<callback_type>::other alloc_callback_t;

    public:
        basic_server(io_service & ios, options const& opts)
            : ios_(ios), acceptor_(ios)
        {
            opts_ = make_shared_ptr<options, alloc_options_t>(opts);
            callback_ = make_shared_ptr<callback_type, alloc_callback_t>();
        }

        ~basic_server()
        {
            terminate();
            while (session_mgr_.size())
                boost::this_thread::sleep( boost::posix_time::milliseconds(1) );
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
            for (std::size_t i = 0; i < listen_count; ++listen_count)
                async_accept();

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

        // ���ûص�
        template <typename session_type::callback_em CallbackType, typename F>
        void set_callback(F const& f)
        {
            session_type::set_callback(*callback_, f);
        }

    private:
        // ���������������
        void async_accept(bool reply = false)
        {
            if (!reply)
            {
                BOOST_INTERLOCKED_INCREMENT(&accept_count_);
            }

            socket_ptr sp = protocol_type::alloc_socket(ios_);
            acceptor_.async_accept(sp->lowest_layer(), 
                boost::bind(&this_type::on_async_accept, this, placeholders::error, sp));
        }

        // ������������ص�
        void on_async_accept(error_code const& ec, socket_ptr sp)
        {
            if (ec)
            {
                if (BOOST_INTERLOCKED_DECREMENT(&accept_count_) == 1 && shutdowning_.is_set())
                    shutdown_sessions();

                return ;
            }

            async_accept(true);

            /// create session
            session_type * session_p = allocate<session_type, alloc_session_t>();
            session_ptr session(session_p, boost::bind(&this_type::session_deleter, this, _1), alloc_session_t());
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
        }

        // ���ŵعر���������
        void shutdown_sessions()
        {
            session_mgr_.for_each(boost::bind(&session_type::shutdown, _1));
        }

        // ǿ�Ƶعر���������
        void terminate_sessions()
        {
            session_mgr_.for_each(boost::bind(&session_type::terminate, _1));
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
    };
    

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SERVER_HPP__