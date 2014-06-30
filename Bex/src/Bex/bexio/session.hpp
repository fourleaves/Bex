#ifndef __BEX_IO_SESSION_HPP__
#define __BEX_IO_SESSION_HPP__

//////////////////////////////////////////////////////////////////////////
/// ����
/*
* @������������Э��
*/
#include "bexio_fwd.hpp"
#include "multithread_strand.hpp"
#include "session_base.hpp"

namespace Bex { namespace bexio
{
    template <typename Protocol, typename SessionMgr>
    class basic_session
        : public Protocol
        , public session_base<SessionMgr>
    {
        typedef basic_session<Protocol, SessionMgr> this_type;
        typedef typename Protocol::allocator allocator;
        typedef typename Protocol::socket_ptr socket_ptr;
        typedef multithread_strand<io_service, allocator> mstrand_service_type;
        friend SessionMgr;

    public:
        typedef Protocol protocol_type;
        typedef SessionMgr session_mgr_type;

        // ����id
        typedef typename session_mgr_type::session_id id;

        /// �ص�����
        enum BEX_ENUM_CLASS callback_em
        {
            cb_connect = 0,         ///< ���ӳɹ�
            cb_disconnect = 1,      ///< �Ͽ�����
            cb_receive = 2,         ///< �յ�����
        };
        typedef callback_em cbe;

        /// ����Protocol::F���id����, �Ƶ���F
        typedef boost::function<void(id)> OnConnectF;
        typedef boost::function<void(id, error_code)> OnDisconnectF;
        typedef typename function_addition<id, typename Protocol::OnReceiveF>::type OnReceiveF;

        typedef boost::tuple<OnConnectF, OnDisconnectF, OnReceiveF> callback_type;

    public:
        explicit basic_session(socket_ptr socket
            , shared_ptr<options> const& opts
            , shared_ptr<callback_type> const& callbacks)

            : opts_(opts), callbacks_(callbacks), socket_(socket), id_(BOOST_INTERLOCKED_INCREMENT(&svlid))
        {
            // initialize sentries
            notify_connect_ = notify_disconnect_ = false;
        }

        void initialize()
        {
            if (opts_->nlte_ == nlte::nlt_reactor)
                mstrand_service()->post(boost::bind(&this_type::notify_onconnect, shared_from_this()));
            else
                notify_connect_ = true;

            post_receive();
        }

        // actor����session����ɻص�
        void run()
        {
            if (opts_->nlte_ != nlte::nlt_loop)
                return ;

            if (notify_connect_)
            {
                notify_onconnect();
                notify_connect_ = false;
            }
            
            // �鿴���յ�������
            on_receive_run();

            if (!receiving_.is_set())
                post_receive();

            if (notify_disconnect_)
            {
                notify_ondisconnect();
                notify_disconnect_ = false;
            }
        }

        // ��������
        bool send(char const* data, std::size_t size)
        {
            if (shutdowning_.is_set())
                return false;

            if (socket_->putable_write() < size)
            {
                /// ���ͻ��������
                if (!on_sendbuffer_overflow())
                    return false;
            }

            socket_->sputn(data, size);
            post_send();
            return true;
        }

        template <typename ConstBufferSequence>
        bool send(ConstBufferSequence const& buffers)
        {
            if (shutdowning_.is_set())
                return false;

            std::size_t size = 0;
            for (typename ConstBufferSequence::iterator it = buffers.begin(); 
                it != buffers.end(); ++it)
                size += buffer_size_helper(*it);

            if (socket_->putable_write() < size)
            {
                /// ���ͻ��������
                if (!on_sendbuffer_overflow())
                    return false;
            }

            socket_->sputn(buffers);
            post_send();
            return true;
        }

        // ���ŵعر�����
        virtual void shutdown()
        {
            on_error(generate_error(bee::initiative_shutdown));
            shutdowning_.set();
        }

        // ǿ�Ƶعر�����
        virtual void terminate()
        {
            on_error(generate_error(bee::initiative_terminate));
            error_code ec;
            socket_->lowest_layer().cancel(ec);
            socket_->lowest_layer().shutdown(socket_base::shutdown_both, ec);
            socket_->close(ec);
            notify_ondisconnect();
        }

        // �����Ƿ��ѶϿ�
        bool is_disconnected() const
        {
            return disconencted_.is_set();
        }

        // ���ûص�
        template <callback_em CallbackType, typename F>
        static void set_callback(callback_type & cb, F const& f)
        {
            boost::get<CallbackType>(cb) = f;
        }

    protected:
        // �����첽��������
        void post_send(bool reply = false)
        {
            if (!reply && !sending_.set())
                return ;

            bool sendok = socket_->async_write_some( post_strand<Protocol>(
                boost::bind(&this_type::on_async_send, shared_from_this(), placeholders::error, placeholders::bytes_transferred)
                ));

            if (!sendok)    ///< ���ͻ������ѿ�
            {
                sending_.reset();

                if (shutdowning_.is_set())   // ��׼���ر�
                {
                    // �رշ���ͨ��
                    close_send();
                }
                else if (socket_->getable_write()) // double check (��ֹ���ͻ������л�������δ�ܼ�ʱ����)
                    post_send();
            }
        }

        // �����첽��������
        void post_receive(bool reply = false)
        {
            if (!reply && !receiving_.set())
                return;

            bool receiveok = socket_->async_read_some( post_strand<Protocol>(
                boost::bind(&this_type::on_async_receive, shared_from_this(), placeholders::error, placeholders::bytes_transferred)
                ));

            if (!receiveok)
            {
                receiving_.reset();

                /// ���ջ���������
                if (on_receivebuffer_overflow())
                    post_receive();
            }
        }

    private:
        // �첽���ͻص�
        void on_async_send(error_code ec, std::size_t bytes)
        {
            if (ec)
            {
                close_send();
                on_error(ec);
                return ;
            }

            socket_->write_done(bytes);
            post_send(true);
        }

        // �첽���ջص�
        void on_async_receive(error_code ec, std::size_t bytes)
        {
            if (ec) 
            {
                close_receive();
                on_error(ec);
                return ;
            }

            socket_->read_done(bytes);

            // on_receive_run�ص�Ҫ��post_receiveǰ��, ����ȷ��waitģʽ�²��˷���Դ.
            if (opts_->nlte_ == nlte::nlt_reactor)
                if (notify_receive_.set())
                    mstrand_service()->post(boost::bind(&this_type::on_receive_run, shared_from_this()));
            post_receive(true);
        }

    private:
        // �������ݻص�(�����߼��߳�ִ��!)
        void on_receive_run()
        {
            notify_receive_.reset();

            boost::array<const_buffer, 2> buffers;
            std::size_t sections = socket_->get_buffers(buffers);
            for (std::size_t i = 0; i < sections; ++i)
            {
                notify_onreceive(buffers[i]);
                socket_->read_done(buffer_size_helper(buffers[i]));
            }

            // ����Ƿ���Ҫ�ر�
            on_error(error_code());
        }

        // ������
        void on_error(error_code ec)
        {
            if (ec && !ec_)  //ֻ���¼��һ������ԭ��
                ec_ = ec;

            /// shutdown is ok?
            if (shutdowning_.is_set())
            {
                if (sendclosed_.is_set() && receiveclosed_.is_set() && !socket_->getable_read())
                {
                    /// ���Թر���
                    error_code ec;
                    socket_->close(ec);
                    notify_ondisconnect();
                }
            }
        }

    private:
        /// �رշ���ͨ��(���ͻ�����������Ϻ���ִ��)
        void close_send()
        {
            sendclosed_.set();
            error_code ec;
            socket_->lowest_layer().shutdown(socket_base::shutdown_send, ec);
        }

        /// �رս���ͨ��(�رպ�Ҫ��������ջ��������ѽ��յ������ݲſ��Թر�)
        void close_receive()
        {
            receiveclosed_.set();
            error_code ec;
            socket_->lowest_layer().shutdown(socket_base::shutdown_receive, ec);
        }

    private:
        /// ֪ͨ�߼��߳�(���߼��߳�ִ��, �ڲ�����֪ͨ������·��)
        void notify_onconnect()
        {
            if (opts_->mlpe_ == mlpe::mlp_derived || opts_->mlpe_ == mlpe::mlp_both)
                on_connect();

            if (opts_->mlpe_ == mlpe::mlp_callback || opts_->mlpe_ == mlpe::mlp_both)
                if (callbacks_ && boost::get<cbe::cb_connect>(callbacks_))
                    boost::get<cbe::cb_connect>(callbacks_)(id(shared_from_this()));
        }

        template <typename ConstBuffer>
        void notify_onreceive(ConstBuffer const& arg)
        {
            if (opts_->mlpe_ == mlpe::mlp_derived || opts_->mlpe_ == mlpe::mlp_both)
                on_receive(arg);
            
            if (opts_->mlpe_ == mlpe::mlp_callback || opts_->mlpe_ == mlpe::mlp_both)
                if (callbacks_ && boost::get<cbe::cb_receive>(callbacks_))
                    boost::get<cbe::cb_receive>(callbacks_)(id(shared_from_this()), arg);
        }

        void notify_ondisconnect()
        {
            disconencted_.set();

            if (opts_->mlpe_ == mlpe::mlp_derived || opts_->mlpe_ == mlpe::mlp_both)
                on_disconnect(ec_);

            if (opts_->mlpe_ == mlpe::mlp_callback || opts_->mlpe_ == mlpe::mlp_both)
                if (callbacks_ && boost::get<cbe::cb_disconnect>(callbacks_))
                    boost::get<cbe::cb_disconnect>(callbacks_)(id(shared_from_this()), ec_);
        }

        /// �������������
        // @return: �Ƿ���Լ�������
        bool on_sendbuffer_overflow()
        {
            if (opts_->sboe_ == sboe::sbo_interrupt)
            {
                on_error(generate_error(bee::sendbuffer_overflow));
                terminate();
                return false;
            }
            else if (opts_->sboe_ == sboe::sbo_wait)
            {
                return false;
            }
            else if (opts_->sboe_ == sboe::sbo_extend)
            {
                // @todo: extend send buffer
                return true;
            }
            else  // default
                return false;
        }

        bool on_receivebuffer_overflow()
        {
            if (opts_->rboe_ == rboe::rbo_interrupt)
            {
                on_error(generate_error(bee::receivebuffer_overflow));
                terminate();
                return false;
            }
            else if (opts_->rboe_ == rboe::rbo_wait)
            {
                if (opts_->nlte_ == nlte::nlt_reactor)
                    mstrand_service()->post(boost::bind(&this_type::post_receive, shared_from_this()));
                return false;
            }
            else if (opts_->rboe_ == rboe::rbo_extend)
            {
                // @todo: extend receive buffer
                return true;
            }
            else // default
                return false;
        }

        mstrand_service_type * mstrand_service()
        {
            if (!service_)
                service_ = &(use_service<mstrand_service_type>(socket_->get_io_service()));

            return service_;
        }

    private:
        /// protocol::socket shared_ptr
        socket_ptr socket_;

        /// multithread_strand service reference.
        mstrand_service_type * service_;

        /// ���������Ƿ���Ͷ��
        sentry<inter_lock> sending_;
        
        /// ���������Ƿ���Ͷ��
        sentry<inter_lock> receiving_;

        /// ����ͨ���Ƿ�ر�
        sentry<bool> sendclosed_;

        /// ����ͨ���Ƿ�ر�
        sentry<bool> receiveclosed_;

        /// �Ƿ����ŵعر���
        // ״̬��־Ч��: 1.����send 2.���ͻ������е����ݷ�����Ϻ�ر�socket����ͨ��
        sentry<bool> shutdowning_;

        /// ������
        // ֻ��¼��һ������ԭ��
        error_code ec_;

        /// �����Ƿ�Ͽ�
        sentry<bool> disconencted_;

        /// ѡ��
        shared_ptr<options> opts_;

        /// �ص�
        shared_ptr<callback_type> callbacks_;

        /// reactor��ʽ, ֪ͨ�߼���receive��Ϣ
        sentry<inter_lock> notify_receive_;

        /// loop��ʽ, ֪ͨconnect/disconnect��Ϣ
        bool notify_connect_;
        bool notify_disconnect_;
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SESSION_HPP__