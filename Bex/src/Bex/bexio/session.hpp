#ifndef __BEX_IO_SESSION_HPP__
#define __BEX_IO_SESSION_HPP__

//////////////////////////////////////////////////////////////////////////
/// ����

/*
* @������������Э��
*
* @functions: ����_cb��_l��׺�Ľӿڱ������߼��߳�ִ�С�
*
* @���ŵعر���������:
*   \������: ����ر�(����shutdown�ӿ�) -> ��ֹ����(sendȫ������false) -> ��¼������
*       -> �ȴ�Ӧ�ò㷢�ͻ���������ȫ�����Ƶ��ײ�socket������ -> �رշ���ͨ��
*       -> �ȴ��Է��ر�����һ��ͨ�� -> ...
*   \������: ȫ�����ݽ������ֱ��eof -> ��¼������ -> mark�������ŵعر� -> ��ֹ����(sendȫ������false)
*       -> �ȴ�Ӧ�ò㷢�ͻ���������ȫ�����Ƶ��ײ�socket������ -> if(mark) �رշ���ͨ��
*       -> do_shutdown_lowest -> �ر����
*   \������: ȫ�����ݽ������ֱ��eof -> do_shutdown_lowest -> �ر����
*
*
* @log 2014-07-10: double pingpong����ʱ, �������ϵ���wait���Ƶ��µ�, ����bug...
*
*/

#include "bexio_fwd.hpp"
#include "multithread_strand_service.hpp"
#include "intrusive_list.hpp"
#include "protocol_traits.hpp"

namespace Bex { namespace bexio
{
    template <class Session>
    struct hook
        : intrusive_list_hook
    {};

    template <typename Protocol,
        template <typename> class Hook = hook
    >
    class basic_session
        : public Protocol
        , public Hook<basic_session<Protocol, Hook> >
        , public boost::enable_shared_from_this<basic_session<Protocol, Hook> >
        , boost::noncopyable
    {
        typedef basic_session<Protocol, Hook> this_type;

    public:
        typedef Protocol protocol_type;
        typedef typename protocol_type::allocator allocator;
        typedef typename protocol_type::socket_ptr socket_ptr;
        typedef typename protocol_type::endpoint endpoint;
        typedef typename protocol_traits<protocol_type> protocol_traits_type;
        typedef multithread_strand_service<allocator> mstrand_service_type;

        // ����id
        class id
            : boost::totally_ordered<id>
        {
            typedef this_type session_type;
            friend session_type;

            explicit id(shared_ptr<session_type> const& spointer)
                : id_(spointer ? spointer->id_ : 0), wpointer_(spointer)
            {}

            friend bool operator<(id const& lhs, id const& rhs)
            {
                return lhs.id_ < rhs.id_;
            }

            shared_ptr<session_type> get()
            {
                return wpointer_.lock();
            }

            long id_;
            weak_ptr<session_type> wpointer_;
        };
        friend class id;

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
        typedef boost::function<void(id, error_code const&)> OnDisconnectF;
        typedef typename function_addition<id, typename Protocol::OnReceiveF>::type OnReceiveF;

        typedef boost::tuple<OnConnectF, OnDisconnectF, OnReceiveF> callback_type;

        typedef shared_ptr<options> options_ptr;
        typedef shared_ptr<callback_type> callback_ptr;

    public:
        // @remarks: Ϊ�û��Զ����������д����, �����ù���ʱ����Ĳ����ӳ���initialize�С�
        //           ��initialize֮ǰsession�ǲ���ȫ��, ǧ��Ҫʹ��!
        basic_session()
            : id_(BOOST_INTERLOCKED_INCREMENT(&svlid)), service_(0)
        {
        }

        void initialize(socket_ptr socket, options_ptr const& opts, callback_ptr const& callbacks)
        {
            opts_ = opts, callbacks_ = callbacks, socket_ = socket;
            protocol_traits_type::initialize(*this, opts_, boost::get<(int)cbe::cb_receive>(*callbacks_), get_id());

            mstrand_service()->post(BEX_IO_BIND(&this_type::do_onconnect_cb, this, shared_this()));

            post_receive();
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
            shutdown_handshaking_.set();
            shutdowning_.set();
            on_error(generate_error(bee::initiative_shutdown));

            BOOST_AUTO(handler, BEX_IO_BIND(&this_type::on_async_shutdown, this, BEX_IO_PH_ERROR, shared_this()));
            if (opts_->ssl_opts)
            {
                BOOST_AUTO(timeout_handler, timer_handler<allocator>(handler, socket_->get_io_service()));
                timeout_handler.expires_from_now(boost::posix_time::milliseconds(opts_->ssl_opts->handshake_overtime));
                timeout_handler.async_wait(BEX_IO_BIND(&this_type::on_async_shutdown, this, generate_error(bee::handshake_overtime), shared_this()));
                protocol_traits_type::async_shutdown(socket_, timeout_handler);
            }
            else
                protocol_traits_type::async_shutdown(socket_, handler);
        }

        // ǿ�ƹر�����(����)
        //   ����RST, �����ᶪʧ�����ϲ�͵ײ�socket���ͻ�������δ���͵�����, 
        //   Ҳ�ᶪʧԶ�˵ײ�socket���ջ�������δ��ȡ�����ݡ�
        virtual void terminate()
        {
            if (!terminating_.set())
                return ;

            on_error(generate_error(bee::initiative_terminate));
            error_code ec;
            socket_->lowest_layer().set_option(socket_base::linger(true, 0), ec);
            do_shutdown_lowest();
        }

        // �����Ƿ��ѶϿ�
        bool is_disconnected() const
        {
            return disconencted_.is_set();
        }

        // ���������ַ
        endpoint local_endpoint() const
        {
            return socket_->lowest_layer().local_endpoint();
        }

        // Զ�������ַ
        endpoint remote_endpoint() const
        {
            return socket_->lowest_layer().remote_endpoint();
        }

        // ��ȡ������Ϣ
        shared_ptr<options const> get_options() const
        {
            return opts_;
        }

        // ��ȡid
        id get_id()
        {
            return id(shared_this());
        }

        // ���ûص�
        template <callback_em CallbackType, typename F>
        static void set_callback(callback_type & cb, F const& f)
        {
            boost::get<CallbackType>(cb) = f;
        }

    protected:
        // ���ӻص�(mlp_derived || mlp_both ��Ч)
        virtual void on_connect() {}
        
        // �Ͽ����ӻص�(mlp_derived || mlp_both ��Ч)
        virtual void on_disconnect(error_code const& ec) {}

    protected:
        // �����첽��������
        void post_send(bool reply = false)
        {
            if (!reply && !sending_.set())
                return ;

            bool sendok = socket_->async_write_some(
                BEX_IO_BIND(&this_type::on_async_send, this, BEX_IO_PH_ERROR, BEX_IO_PH_BYTES_TRANSFERRED, shared_this())
                );

            if (!sendok)    ///< ���ͻ������ѿ�
            {
                sending_.reset();

                if (shutdowning_.is_set() 
                    && !shutdown_handshaking_.is_set()
                    && !socket_->getable_write())   // ��׼���ر�
                {
                    close_send();   // �رշ���ͨ��
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

            bool receiveok = socket_->async_read_some(
                BEX_IO_BIND(&this_type::on_async_receive, this, BEX_IO_PH_ERROR, BEX_IO_PH_BYTES_TRANSFERRED, shared_this())
                );

            if (!receiveok)
            {
                receiving_.reset();

                /// ���ջ���������
                if (on_receivebuffer_overflow())
                    post_receive();
            }
        }
        // ֪ͨ�߼��̷߳����첽��������Ľӿ�
        void post_receive_cb(shared_ptr<this_type>)
        {
            post_receive();
        }

    private:
        // �첽���ͻص�
        void on_async_send(error_code ec, std::size_t bytes, shared_ptr<this_type>)
        {
            //Dump("on_async_send, ec:" << ec.value() << " bytes:" << bytes);

            if (ec)
            {
                close_send();
                on_error(ec);
                return ;
            }

            post_send(true);
        }

        // �첽���ջص�
        void on_async_receive(error_code ec, std::size_t bytes, shared_ptr<this_type>)
        {
            //Dump("on_async_receive, ec:" << ec.value() << " bytes:" << bytes);

            if (ec) 
            {
                close_receive();

                // eof �Է�����shutdown, ���������ŵط�ʽ�ر�����.
                if (ec.value() == 2)
                {
                    shutdowning_.set();
                    on_error(generate_error(bee::passive_shutdown));
                }
                else
                    on_error(ec);

                return ;
            }

            // on_receive_run�ص�Ҫ��post_receiveǰ��, ����ȷ��waitģʽ�²��˷���Դ.
            if (notify_receive_.set())
                mstrand_service()->post(BEX_IO_BIND(&this_type::on_receive_cb, this, shared_this()));

            post_receive(true);
        }

        // �ر����ֻص�
        void on_async_shutdown(error_code const& ec, shared_ptr<this_type>)
        {
            shutdown_handshaking_.reset();
            if (ec)
                do_shutdown_lowest();
            else
            {
                shutdown_dt_ = make_shared_ptr<deadline_timer, allocator>(socket_->get_io_service());
                shutdown_dt_->expires_from_now(boost::posix_time::milliseconds(opts_->shutdown_timeout));
                shutdown_dt_->async_wait(BEX_IO_BIND(&this_type::on_shutdown_overtime, this, BEX_IO_PH_ERROR, shared_from_this()));
                if (!sending_.is_set() && !socket_->getable_write())
                    close_send();
            }
        }

        // �ر�����
        void do_shutdown_lowest()
        {
            if (shutdown_dt_)
            {
                error_code ec;
                shutdown_dt_->cancel(ec);
                shutdown_dt_.reset();
            }

            error_code ec;
            socket_->lowest_layer().cancel(ec);
            socket_->lowest_layer().shutdown(socket_base::shutdown_both, ec);
            socket_->lowest_layer().close(ec);
            notify_ondisconnect();
        }

        // shutdown��ʱ��ֱ�ӹر�����
        void on_shutdown_overtime(error_code const& ec, shared_ptr<this_type>)
        {
            if (ec)
                return ;

            shutdown_dt_.reset();
            ec_ = generate_error(bee::shutdown_overtime);   // ǿ���޸ĶϿ�����ԭ��
            terminate();
        }

    private:
        // �������ݻص�(�����߼��߳�ִ��!)
        void on_receive_l()
        {
            boost::array<const_buffer, 2> buffers;
            std::size_t sections = socket_->get_buffers(buffers);
            for (std::size_t i = 0; i < sections; ++i)
            {
                do_onreceive_l(buffers[i]);
                socket_->read_done(buffer_size_helper(buffers[i]));
            }
        }
        void on_receive_cb(shared_ptr<this_type>)
        {
            notify_receive_.reset();
            on_receive_l();
        }

        // ������
        void on_error(error_code ec)
        {
            if (ec && !ec_)  //ֻ���¼��һ������ԭ��
                ec_ = ec;

            /// shutdown is ok?
            if (shutdowning_.is_set())
            {
                if (receiveclosed_.is_set())
                {
                    if (sendclosed_.is_set())   //����ͨ���ѹر�, ���Թر�socket��.
                        do_shutdown_lowest();
                    else if (!sending_.is_set())    //������ֹͣ, ���Թرշ���ͨ����.
                        close_send();
                    else if (!socket_->getable_write())  //����ͨ��δ�رյ����ͻ������ѿ�, ���Թرշ���ͨ����.
                        close_send();
                }
            }
            else if (ec && terminating_.set())
            {
                do_shutdown_lowest();
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
        void do_onconnect_l()
        {
            if (opts_->mlpe_ == mlpe::mlp_derived || opts_->mlpe_ == mlpe::mlp_both)
                on_connect();

            if (opts_->mlpe_ == mlpe::mlp_callback || opts_->mlpe_ == mlpe::mlp_both)
                if (callbacks_ && boost::get<(int)cbe::cb_connect>(*callbacks_))
                    boost::get<(int)cbe::cb_connect>(*callbacks_)(get_id());
        }
        void do_onconnect_cb(shared_ptr<this_type>)
        {
            do_onconnect_l();
        }

        template <typename ConstBuffer>
        void do_onreceive_l(ConstBuffer const& arg)
        {
            protocol_type::parse(arg);
        }

        void do_ondisconnect_l()
        {
            disconencted_.set();

            if (opts_->mlpe_ == mlpe::mlp_derived || opts_->mlpe_ == mlpe::mlp_both)
                on_disconnect(ec_);

            if (opts_->mlpe_ == mlpe::mlp_callback || opts_->mlpe_ == mlpe::mlp_both)
                if (callbacks_ && boost::get<(int)cbe::cb_disconnect>(*callbacks_))
                    boost::get<(int)cbe::cb_disconnect>(*callbacks_)(get_id(), ec_);
        }
        void do_ondisconnect_cb(shared_ptr<this_type> )
        {
            do_ondisconnect_l();
        }
        void notify_ondisconnect()
        {
            if (!disconect_notified_.set())
                return ;

            mstrand_service()->post(BEX_IO_BIND(&this_type::do_ondisconnect_cb, this, shared_this()));
        }

        /// �������������
        // @return: �Ƿ���Լ�������
        bool on_sendbuffer_overflow()
        {
            if (opts_->sboe_ == sboe::sbo_interrupt)
            {
                on_error(generate_error(bee::sendbuffer_overflow));
                return false;
            }
            else if (opts_->sboe_ == sboe::sbo_wait)
            {
                return false;
            }
            //else if (opts_->sboe_ == sboe::sbo_extend)
            //{
            //    // @todo: extend send buffer
            //    return true;
            //}
            else  // default
                return false;
        }

        bool on_receivebuffer_overflow()
        {
            if (opts_->rboe_ == rboe::rbo_interrupt)
            {
                on_error(generate_error(bee::receivebuffer_overflow));
                return false;
            }
            else if (opts_->rboe_ == rboe::rbo_wait)
            {
                mstrand_service()->post(BEX_IO_BIND(&this_type::post_receive_cb, this, shared_this()));
                return false;
            }
            //else if (opts_->rboe_ == rboe::rbo_extend)
            //{
            //    // @todo: extend receive buffer
            //    return true;
            //}
            else // default
                return false;
        }

        mstrand_service_type * mstrand_service()
        {
            if (!service_)
                service_ = &(use_service<mstrand_service_type>(socket_->get_io_service()));

            return service_;
        }

        shared_ptr<this_type> shared_this()
        {
            return boost::enable_shared_from_this<basic_session<Protocol, Hook> >::shared_from_this();
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

        /// �����Ƿ���֪ͨ
        sentry<bool> disconect_notified_;

        /// ����ͨ���Ƿ�ر�
        sentry<bool> sendclosed_;

        /// ����ͨ���Ƿ�ر�
        sentry<bool> receiveclosed_;

        /// �Ƿ����ŵعر���
        // ״̬��־Ч��: 1.����send 2.���ͻ������е����ݷ�����Ϻ�ر�socket����ͨ��
        sentry<bool> shutdowning_;

        /// �ر�������
        sentry<bool> shutdown_handshaking_;

        /// ���ŵعرճ�ʱ��ʱ��
        shared_ptr<deadline_timer> shutdown_dt_;

        /// �Ƿ�ǿ�ƹر���
        sentry<inter_lock> terminating_;

        /// ������
        // ֻ��¼��һ������ԭ��
        error_code ec_;

        /// �����Ƿ�Ͽ�
        sentry<bool> disconencted_;

        /// ѡ��
        options_ptr opts_;

        /// �ص�
        callback_ptr callbacks_;

        /// reactor��ʽ, ֪ͨ�߼���receive��Ϣ
        sentry<inter_lock> notify_receive_;

        /// session id
        long id_;
        static volatile long svlid;
    };

    template <typename Protocol,
        template <typename> class Hook
    >
    volatile long basic_session<Protocol, Hook>::svlid = 1;

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_SESSION_HPP__