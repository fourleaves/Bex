#ifndef __BEX_IO_OPTIONS_HPP__
#define __BEX_IO_OPTIONS_HPP__

//////////////////////////////////////////////////////////////////////////
/// ��ѡ������ options
#include <Bex/config.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>

namespace Bex { namespace bexio
{
    using namespace boost::asio;

    //////////////////////////////////////////////////////////////////////////
    /// @{ ѡ��
    // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ֪ͨ�߼��̵߳ķ�ʽ
    enum BEX_ENUM_CLASS notify_logic_thread_em
    {
        nlt_reactor,        ///< reactorģʽ, ����Ϣpost��ָ����ɶ�����.
        nlt_loop,           ///< �߼��߳���ѯsession
    };
    typedef notify_logic_thread_em nlte;

    // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ�߼��̵߳���Ӧ��ʽ
    enum BEX_ENUM_CLASS message_logic_process_em
    {
        mlp_callback,       ///< ע��ص�����
        mlp_derived,        ///< �̳�sessioin����д��Ϣ��Ӧ����
        mlp_both,           ///< ���з�ʽһ����Ч
    };
    typedef message_logic_process_em mlpe;

    // ���ͻ��������������
    enum BEX_ENUM_CLASS send_buffer_overflow_em
    {
        sbo_interrupt,      ///< �Ͽ�����
        sbo_wait,           ///< �߼���ȴ�
        sbo_extend,         ///< ��չ���ͻ�����(�ݲ�֧��)
    };
    typedef send_buffer_overflow_em sboe;

    // ���ջ��������������
    enum BEX_ENUM_CLASS receive_buffer_overflow_em
    {
        rbo_interrupt,      ///< �Ͽ�����
        rbo_wait,           ///< ���ղ�ȴ�
        rbo_extend,         ///< ��չ���ջ�����(�ݲ�֧��)
    };
    typedef receive_buffer_overflow_em rboe;
    /// @}
    //////////////////////////////////////////////////////////////////////////

    struct ssl_options
    {
        // ������ѡ��
        // * Ĭ��ֵ: boost::asio::ssl::context_base::default_workarounds | boost::asio::ssl::context_base::no_sslv2
        // * ʹ�÷���:
        //      @begin code:
        //          ctx.set_options(ctx_opt);
        //      @end code.
        long ctx_opt;

        // ��֤Ҫ��
        // * Ĭ��ֵ: boost::asio::ssl::verify_none
        // * ʹ�÷���:
        //      @begin code:
        //          ctx.set_verify_mode(verify_mode);
        //      @end code.
        long verify_mode;

        // ֤���ļ� (server�˱���, client�˿�ѡ)
        // * Ĭ��ֵ: "server.pem"
        // * ʹ�÷���:
        //      @begin code:
        //          ctx.use_certificate_chain_file(crt_file);
        //      @end code.
        std::string crt_file;

        // ˽Կ�ļ� & ˽Կ�ļ�����
        // * Ĭ��ֵ: "server.pem", boost::asio::ssl::context_base::pem
        // * ʹ�÷���:
        //      @begin code:
        //          ctx.use_private_key_file(pri_key_file.c_str(), file_fmt);
        //      @end code.
        std::string pri_key_file;
        ssl::context_base::file_format file_fmt;

        // dh�㷨��Կ�ļ�
        // * Ĭ��ֵ: "dh512.pem"
        // * ʹ�÷���:
        //      @begin code:
        //          ctx.use_tmp_dh_file(dh_file);
        //      @end code.
        std::string dh_file;

        // �����㷨��
        // * Ĭ��ֵ: "RC4-MD5"
        // * ʹ�÷���:
        //      @begin code:
        //    SSL_CTX_set_cipher_list(ctx.native_handle(), cipher_list.c_str());
        //      @end code.
        std::string cipher_list;

        static ssl_options client()
        {
            static ssl_options opts = {
                ssl::context_base::default_workarounds | ssl::context_base::no_sslv2,
                ssl::verify_none,
                "",
                "",
                ssl::context_base::pem,
                "dh512.pem",
                "RC4-MD5"
            };
            return opts;
        }

        static ssl_options server()
        {
            static ssl_options opts = {
                ssl::context_base::default_workarounds | ssl::context_base::no_sslv2,
                ssl::verify_none,
                "bexio.crt",
                "bexio.pem",
                ssl::context_base::pem,
                "dh512.pem",
                "RC4-MD5"
            };
            return opts;
        }
        //////////////////////////////////////////////////////////////////////////
        /// @{ sample
        // @begin code: client peer
        //
        //    ctx.set_options(boost::asio::ssl::context::default_workarounds
        //        | boost::asio::ssl::context::no_sslv2);
        //    ctx.set_verify_mode(boost::asio::ssl::verify_none);
        //
        // @end code;
        //
        // @begin code: server peer
        //
        //    ctx.set_options(boost::asio::ssl::context::default_workarounds
        //        | boost::asio::ssl::context::no_sslv2
        //        /*| boost::asio::ssl::context::single_dh_use*/ );
        //
        //    ctx.use_certificate_chain_file("server.pem");
        //    ctx.use_private_key_file("server.pem", boost::asio::ssl::context::pem);
        //    SSL_CTX_set_cipher_list(ctx.native_handle(), "RC4-MD5");
        //
        // @end code;
        /// @}
        //////////////////////////////////////////////////////////////////////////
    };

    struct options
    {
        // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ֪ͨ�߼��̵߳ķ�ʽ
        notify_logic_thread_em nlte_;

        // ���ͻ��������������
        send_buffer_overflow_em sboe_;

        // ���ջ��������������
        receive_buffer_overflow_em rboe_;

        // ���ӳɹ�\�Ͽ�����\�յ����� ������Ϣ�߼��̵߳���Ӧ��ʽ
        message_logic_process_em mlpe_;

        // �����߳�����(0��ʾ�����������߳���)
        std::size_t workthread_count;

        // ���ͻ�����
        std::size_t send_buffer_size;
        static const std::size_t default_sbsize = 1024 * 8;
        static const std::size_t large_sbsize = 1024 * 1024 * 8;

        // ���ͻ�����
        std::size_t receive_buffer_size;
        static const std::size_t default_rbsize = 1024 * 8;
        static const std::size_t large_rbsize = 1024 * 8;

        // ÿ�������󳤶�(Ŀǰ��tcp packet/ssl packetЭ����Ч)
        std::size_t max_packet_size;
        static const std::size_t default_max_packet_size = 1024 * 8;

        /// ssl����
        ssl_options ssl_opts;

        /// �������÷���(Test)
        static options test()
        {
            static options const opts = {
                nlte::nlt_reactor,
                sboe::sbo_wait,
                rboe::rbo_wait,
                mlpe::mlp_derived,
                0,
                default_sbsize,
                default_rbsize,
                default_max_packet_size
                };
            return opts;
        }

        /// �߲�������������������Ƽ����÷���
        static options multi_session_server()
        {
            static options const opts = {
                nlte::nlt_reactor,
                sboe::sbo_interrupt,
                rboe::rbo_interrupt,
                mlpe::mlp_derived,
                0,
                default_sbsize,
                default_rbsize,
                default_max_packet_size
                };
            return opts;
        }

        /// �ȶ������ȵ��Ƽ����÷���
        static options stability()
        {
            static options const opts = {
                nlte::nlt_reactor,
                sboe::sbo_extend,
                rboe::rbo_extend,
                mlpe::mlp_derived,
                0,
                default_sbsize,
                default_rbsize,
                default_max_packet_size
                };
            return opts;
        }

        /// ���������������������Ƽ����÷���(�ͻ��˺��ʺ�ʹ�����)
        static options throughput()
        {
            static options const opts = {
                nlte::nlt_loop,
                sboe::sbo_extend,
                rboe::rbo_extend,
                mlpe::mlp_derived,
                0,
                large_sbsize,
                large_rbsize,
                default_max_packet_size
                };
            return opts;
        }
    };

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_OPTIONS_HPP__
