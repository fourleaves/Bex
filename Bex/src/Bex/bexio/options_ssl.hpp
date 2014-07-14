#ifndef __BEX_IO_OPTIONS_SSL_HPP__
#define __BEX_IO_OPTIONS_SSL_HPP__

//////////////////////////////////////////////////////////////////////////
/// ssl��ѡ������
#include <Bex/config.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#include <string>

namespace Bex { namespace bexio
{
    using namespace boost::asio;

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

} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_OPTIONS_SSL_HPP__